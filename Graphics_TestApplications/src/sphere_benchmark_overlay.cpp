/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

// spheres_benchmark.cpp
// Scenario: Many spheres, adjustable object count and quality.
// Compares OpenGL ES2 and Vulkan backends on the same Wayland surface.
//
// Vulkan path assumes external SPIR-V shader binaries:
//   sphere_vert.spv  (vertex shader)
//   sphere_frag.spv  (fragment shader)
//
// Build (example):
//   g++ spheres_benchmark.cpp -o spheres_bench -std=c++17 \
//       -lwayland-client -lwayland-egl -lEGL -lGLESv2 -lvulkan
//
// Usage examples:
//   ./spheres_bench --api=gl     --objects=500 --quality=2 --cpu-work=1 --fps=60
//   ./spheres_bench --api=vulkan --objects=500 --quality=2 --cpu-work=1 --fps=60

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <unistd.h>
#include <thread>
#include <stdexcept>
#include "overlay.h"
#include <simpleshell-client-protocol.h>

// ----------------- Args / utility -----------------

struct Args {
    enum class API { GL, Vulkan } api = API::GL;
    int frames      = 1000;
    int width       = 1920;
    int height      = 1080;
    int numObjects  = 100;   // number of spheres
    int quality     = 2;     // 1=low, 2=med, 3=high
    int cpuWork     = 1;     // 0=light,1=medium,2=heavy
    int targetFPS   = 60;    // 0 = uncapped
    std::string csvFile = "spheres_bench.csv";
};

int MOTION_WINDOW_WIDTH = 1920;
int MOTION_WINDOW_HEIGHT = 1080;

Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--api=gl") == 0) a.api = Args::API::GL;
        else if (strcmp(argv[i], "--api=vulkan") == 0) a.api = Args::API::Vulkan;
        else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) a.frames = std::atoi(argv[++i]);
        else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) { a.width = std::atoi(argv[++i]); MOTION_WINDOW_WIDTH = a.width; }
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) { a.height = std::atoi(argv[++i]); MOTION_WINDOW_HEIGHT = a.height; }
        else if (strcmp(argv[i], "--objects") == 0 && i + 1 < argc) a.numObjects = std::atoi(argv[++i]);
        else if (strcmp(argv[i], "--quality") == 0 && i + 1 < argc) a.quality = std::atoi(argv[++i]);
        else if (strcmp(argv[i], "--cpu-work") == 0 && i + 1 < argc) a.cpuWork = std::atoi(argv[++i]);
        else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) a.targetFPS = std::atoi(argv[++i]);
        else if (strcmp(argv[i], "--csv") == 0 && i + 1 < argc) a.csvFile = argv[++i];
    }
    return a;
}

// CPU usage
static unsigned long long prevTotal = 0;
static unsigned long long prevIdle = 0;

double getCPUUsage() {
    std::ifstream f("/proc/stat");
    if (!f.is_open()) return 0.0;
    std::string cpu;
    unsigned long long user=0, nice=0, system=0, idle=0, iowait=0, irq=0, softirq=0, steal=0;
    f >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    unsigned long long idleAll = idle + iowait;
    unsigned long long nonIdle = user + nice + system + irq + softirq + steal;
    unsigned long long total = idleAll + nonIdle;
    double cpuPercent = 0.0;
    if (prevTotal != 0) {
        unsigned long long totald = total - prevTotal;
        unsigned long long idled = idleAll - prevIdle;
        if (totald != 0) cpuPercent = (double)(totald - idled) / (double)totald * 100.0;
    }
    prevTotal = total;
    prevIdle = idleAll;
    return cpuPercent;
}

void appendCSV(const std::string &file, const std::string &api,
               int numObjects, int quality, int cpuWork,
               double fps, double cpuEnd)
{
    bool exists = (access(file.c_str(), F_OK) == 0);
    std::ofstream out(file, std::ios::app);
    if (!exists) {
        out << "API,Resolution,Objects,Quality,CPUWork,FPS,CPU_Usage\n";
    }
    out << api << "," << MOTION_WINDOW_WIDTH << "x" << MOTION_WINDOW_HEIGHT << "," << numObjects << "," << quality << "," << cpuWork << ","
        << fps << "," << cpuEnd << "\n";
}

inline void capFrameTime(const Args &args,
                         const std::chrono::high_resolution_clock::time_point &frameStart)
{
    if (args.targetFPS <= 0) return;
    using namespace std::chrono;
    double targetSeconds = 1.0 / (double)args.targetFPS;
    auto now = high_resolution_clock::now();
    double elapsed = duration<double>(now - frameStart).count();
    double remaining = targetSeconds - elapsed;
    if (remaining > 0.0) {
        std::this_thread::sleep_for(duration<double>(remaining));
    }
}

// ----------------- Wayland -----------------

struct WaylandApp {
    wl_display    *display    = nullptr;
    wl_registry   *registry   = nullptr;
    wl_compositor *compositor = nullptr;
    wl_surface    *surface    = nullptr;
	wl_simple_shell *shell    = nullptr; 
    wl_egl_window *egl_window = nullptr;
	wl_shm* shm = nullptr;
};

static void registry_global(void *data, wl_registry *registry,
                            uint32_t id, const char *interface, uint32_t)
{
    WaylandApp *app = (WaylandApp*)data;

    if (!strcmp(interface, "wl_compositor"))
        app->compositor = (wl_compositor*)
            wl_registry_bind(registry, id, &wl_compositor_interface, 1);

    else if (!strcmp(interface, "wl_shm"))
        app->shm = (wl_shm*)
            wl_registry_bind(registry, id, &wl_shm_interface, 1);

    else if (!strcmp(interface, "wl_simple_shell"))
        app->shell = (wl_simple_shell*)
            wl_registry_bind(registry, id, &wl_simple_shell_interface, 1);
}


static void registry_global_remove(void *data, wl_registry *registry, uint32_t id) {
    (void)data; (void)registry; (void)id;
}

bool initWayland(WaylandApp &app) {
    app.display = wl_display_connect(nullptr);
    if (!app.display) {
        std::cerr << "[Wayland] wl_display_connect failed\n";
        return false;
    }

    app.registry = wl_display_get_registry(app.display);

    static const wl_registry_listener reg_listener = {
        registry_global, registry_global_remove
    };
    wl_registry_add_listener(app.registry, &reg_listener, &app);

    wl_display_roundtrip(app.display);

    /* Now validate */
    if (!app.compositor || !app.shell || !app.shm) {
        std::cerr << "[Wayland] Missing required globals:\n";
        std::cerr << "  compositor: " << app.compositor << "\n";
        std::cerr << "  shell:      " << app.shell << "\n";
        std::cerr << "  shm:        " << app.shm << "\n";
        return false;
    }

    app.surface = wl_compositor_create_surface(app.compositor);
    if (!app.surface) {
        std::cerr << "[Wayland] wl_compositor_create_surface failed\n";
        return false;
    }

    return true;
}

// ----------------- Simple math / sphere generation -----------------

struct Vec3 { float x,y,z; };
struct Mat4 { float m[16]; };

Mat4 matIdentity() {
    Mat4 r{};
    for (int i=0;i<16;++i) r.m[i]=0.0f;
    r.m[0]=r.m[5]=r.m[10]=r.m[15]=1.0f;
    return r;
}

Mat4 matPerspective(float fovyRadians, float aspect, float znear, float zfar) {
    float f = 1.0f/std::tan(fovyRadians*0.5f);
    Mat4 r{};
    for (int i=0;i<16;++i) r.m[i]=0;
    r.m[0] = f/aspect;
    r.m[5] = f;
    r.m[10] = (zfar+znear)/(znear-zfar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f*zfar*znear)/(znear-zfar);
    return r;
}

Mat4 matTranslate(const Vec3 &v) {
    Mat4 r = matIdentity();
    r.m[12]=v.x; r.m[13]=v.y; r.m[14]=v.z;
    return r;
}

Mat4 matRotateY(float angle) {
    Mat4 r = matIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    r.m[0]=c;  r.m[2]=s;
    r.m[8]=-s; r.m[10]=c;
    return r;
}

Mat4 matMul(const Mat4 &a, const Mat4 &b) {
    Mat4 r{};
    for (int row=0;row<4;++row) {
        for (int col=0;col<4;++col) {
            r.m[row*4+col] =
                a.m[row*4+0]*b.m[0*4+col] +
                a.m[row*4+1]*b.m[1*4+col] +
                a.m[row*4+2]*b.m[2*4+col] +
                a.m[row*4+3]*b.m[3*4+col];
        }
    }
    return r;
}

// Sphere mesh
struct SphereMesh {
    std::vector<float> vertices; // pos(3)+normal(3)
    std::vector<unsigned short> indices;
};

SphereMesh generateSphere(int stacks, int slices) {
    SphereMesh mesh;
    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / (float)stacks;
        float phi = v * M_PI;
        float y = std::cos(phi);
        float r = std::sin(phi);
        for (int j = 0; j <= slices; ++j) {
            float u = (float)j / (float)slices;
            float theta = u * 2.0f * M_PI;
            float x = r * std::cos(theta);
            float z = r * std::sin(theta);
            mesh.vertices.push_back(x);
            mesh.vertices.push_back(y);
            mesh.vertices.push_back(z);
            mesh.vertices.push_back(x);
            mesh.vertices.push_back(y);
            mesh.vertices.push_back(z);
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int row1 = i * (slices+1);
            int row2 = (i+1) * (slices+1);
            unsigned short a = (unsigned short)(row1 + j);
            unsigned short b = (unsigned short)(row1 + j + 1);
            unsigned short c = (unsigned short)(row2 + j);
            unsigned short d = (unsigned short)(row2 + j + 1);
            mesh.indices.push_back(a);
            mesh.indices.push_back(c);
            mesh.indices.push_back(b);
            mesh.indices.push_back(b);
            mesh.indices.push_back(c);
            mesh.indices.push_back(d);
        }
    }
    return mesh;
}

SphereMesh makeQualitySphere(int quality) {
    if (quality <= 1) return generateSphere(8,8);
    if (quality == 2) return generateSphere(16,16);
    return generateSphere(32,32);
}

// Per-object data
struct Object {
    Vec3  position;
    float baseAngle;
    float rotationSpeed;
    Vec3  color;
};

std::vector<Object> createObjects(int numObjects) {
    std::vector<Object> objs;
    objs.reserve(numObjects);
    int perRow = (int)std::floor(std::sqrt((float)numObjects));
    if (perRow < 1) perRow = 1;
    float spacing = 1.8f;
    int row = 0, col = 0;
    for (int i = 0; i < numObjects; ++i) {
        float x = (col - perRow*0.5f) * spacing;
        float z = (row - perRow*0.5f) * spacing - 10.0f;
        float y = 0.0f;
        Object o{};
        o.position = {x,y,z};
        o.baseAngle = (float)i * 0.1f;
        o.rotationSpeed = 0.5f + 0.1f * (float)(i % 5);
        o.color = { (float)(i % 5)/4.0f, (float)((i/5)%5)/4.0f, 0.5f };
        objs.push_back(o);
        ++col;
        if (col >= perRow) { col = 0; ++row; }
    }
    return objs;
}

// CPU work
inline void doCPUWork(const Args &args, const Object &o, int frame) {
    if (args.cpuWork <= 0) return;
    volatile float acc = 0.0f;
    int iterations = (args.cpuWork == 1) ? 50 : 200;
    for (int i = 0; i < iterations; ++i) {
        float t = (float)frame * 0.001f + (float)i * 0.0001f + o.baseAngle;
        acc += std::sin(t) * std::cos(t*0.5f) * 0.5f;
    }
    (void)acc;
}

// ----------------- GLSL for GL backend -----------------

const char *vsSource = R"(attribute vec3 aPos;
attribute vec3 aNormal;

uniform mat4 uMVP;
uniform vec3 uColor;

varying vec3 vNormal;
varying vec3 vColor;

void main() {
    vNormal = aNormal;
    vColor = uColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

const char *fsSource = R"(precision mediump float;
varying vec3 vNormal;
varying vec3 vColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(0.5, 1.0, 0.2));
    float diff = max(dot(N, L), 0.0);
    vec3 color = vColor * (0.2 + 0.8 * diff);
    gl_FragColor = vec4(color, 1.0);
}
)";

GLuint compileShader(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = GL_FALSE;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
        std::cerr << "[GL] Shader compile error: " << buf << "\n";
    }
    return s;
}

GLuint createProgram(const char *vs, const char *fs) {
    GLuint vsId = compileShader(GL_VERTEX_SHADER, vs);
    GLuint fsId = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsId);
    glAttachShader(prog, fsId);
    glBindAttribLocation(prog, 0, "aPos");
    glBindAttribLocation(prog, 1, "aNormal");
    glLinkProgram(prog);
    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[1024];
        glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        std::cerr << "[GL] Program link error: " << buf << "\n";
    }
    glDeleteShader(vsId);
    glDeleteShader(fsId);
    return prog;
}

// ----------------- OpenGL ES2 backend -----------------

void runGLSpheres(const Args &args) {
    WaylandApp wl{};
    if (!initWayland(wl)) return;
	
    wl.egl_window = wl_egl_window_create(wl.surface, args.width, args.height);
    if (!wl.egl_window) {
        std::cerr << "[GL] wl_egl_window_create failed\n";
        return;
    }

    EGLDisplay eglDisplay = eglGetDisplay((EGLNativeDisplayType)wl.display);
    if (eglDisplay == EGL_NO_DISPLAY)
        eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        std::cerr << "[GL] eglInitialize failed\n";
        return;
    }

    EGLint cfgAttrs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLConfig config;
    EGLint numCfg = 0;
    if (!eglChooseConfig(eglDisplay, cfgAttrs, &config, 1, &numCfg) || numCfg == 0) {
        std::cerr << "[GL] eglChooseConfig failed\n";
        return;
    }

    EGLint ctxAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLContext ctx = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxAttrs);
    if (ctx == EGL_NO_CONTEXT) {
        std::cerr << "[GL] eglCreateContext failed\n";
        return;
    }

    EGLSurface surf = eglCreateWindowSurface(
        eglDisplay, config,
        (EGLNativeWindowType)wl.egl_window, nullptr);
    if (surf == EGL_NO_SURFACE) {
        std::cerr << "[GL] eglCreateWindowSurface failed\n";
        return;
    }

    if (!eglMakeCurrent(eglDisplay, surf, surf, ctx)) {
        std::cerr << "[GL] eglMakeCurrent failed\n";
        return;
    }
	
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    SphereMesh sphere = makeQualitySphere(args.quality);
    std::cout << "[GL] Sphere vertices: " << sphere.vertices.size()/6
              << " indices: " << sphere.indices.size() << "\n";

    GLuint vbo=0, ibo=0;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sphere.vertices.size()*sizeof(float),
                 sphere.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sphere.indices.size()*sizeof(unsigned short),
                 sphere.indices.data(), GL_STATIC_DRAW);

    GLuint prog = createProgram(vsSource, fsSource);
    glUseProgram(prog);
    GLint locMVP   = glGetUniformLocation(prog, "uMVP");
    GLint locColor = glGetUniformLocation(prog, "uColor");

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (const void*)(3*sizeof(float)));

    std::vector<Object> objects = createObjects(args.numObjects);
    std::cout << "[GL] Objects: " << objects.size() << "\n";

    Mat4 proj = matPerspective(45.0f * (3.1415926f/180.0f),
                               (float)args.width / (float)args.height,
                               0.1f, 100.0f);

    getCPUUsage();
    double cpuStart = getCPUUsage();
    auto start = std::chrono::high_resolution_clock::now();
    auto lastReportTime = std::chrono::high_resolution_clock::now();
    int framesThisSecond = 0;
    double lastCPU = getCPUUsage();

	bool overlayReady = false;
	
    for (int frame = 0; frame < args.frames; ++frame) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        glViewport(0, 0, args.width, args.height);
        glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float timeSec = (float)std::chrono::duration<double>(
            frameStart - start).count();

        for (int i = 0; i < (int)objects.size(); ++i) {
            Object &o = objects[i];

            doCPUWork(args, o, frame);

            float angle = o.baseAngle + o.rotationSpeed * timeSec;
            Mat4 model = matMul(matTranslate(o.position), matRotateY(angle));
            Mat4 mvp   = matMul(proj, model);

            glUniformMatrix4fv(locMVP, 1, GL_FALSE, mvp.m);
            glUniform3f(locColor, o.color.x, o.color.y, o.color.z);

            glDrawElements(GL_TRIANGLES,
                           (GLsizei)sphere.indices.size(),
                           GL_UNSIGNED_SHORT,
                           (const void*)0);
        }

        eglSwapBuffers(eglDisplay, surf);
		

		
		if (!overlayReady) {
			overlayInit(
                            wl.display,
                            wl.compositor,
                            wl.shell,
                            eglDisplay,
							ctx,
                            args.width,
                            args.height
                        );
			overlayShowInfo(
				"MOTION_BENCH",
				"OPENGL",
				args.numObjects
			);
			overlayReady = true;
		
		}
	framesThisSecond++;

	auto now = std::chrono::high_resolution_clock::now();
	double elapsed =
	    std::chrono::duration<double>(now - lastReportTime).count();

	if (elapsed >= 1.0) {
	    double currentCPU = getCPUUsage();

	    double fpsThisSecond = framesThisSecond / elapsed;
	    double cpuDelta = currentCPU - lastCPU;

	    std::cout << "[GL] FPS: " << fpsThisSecond
	              << " CPU: " << currentCPU << "% "
	              << "(Δ " << cpuDelta << "%)"
        	      << std::endl;

        overlayUpdatePerf(fpsThisSecond, currentCPU);
	    appendCSV(args.csvFile, "OpenGL", args.numObjects, args.quality, args.cpuWork,
              fpsThisSecond, currentCPU);
	    // reset for next interval
	    framesThisSecond = 0;
	    lastReportTime = now;
	    lastCPU = currentCPU;
	}
        capFrameTime(args, frameStart);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double cpuEnd = getCPUUsage();
    double secs = std::chrono::duration<double>(end - start).count();
    double fps = args.frames / (secs > 0.0 ? secs : 1.0);

    std::cout << "[GL] FPS: " << fps
              << "% CPU End: " << cpuEnd << "%\n";
    appendCSV(args.csvFile, "OpenGL", args.numObjects, args.quality, args.cpuWork,
              fps, cpuEnd);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteProgram(prog);

    overlayShutdown();

    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(eglDisplay, surf);
    eglDestroyContext(eglDisplay, ctx);
    //eglTerminate(eglDisplay);

	
    wl_egl_window_destroy(wl.egl_window);
    if (wl.surface)    wl_proxy_destroy((wl_proxy*)wl.surface);
    if (wl.compositor) wl_proxy_destroy((wl_proxy*)wl.compositor);
    if (wl.registry)   wl_proxy_destroy((wl_proxy*)wl.registry);
    if (wl.display)    wl_display_disconnect(wl.display);
}

// ----------------- Vulkan backend -----------------

struct VulkanContext {
    VkInstance       instance        = VK_NULL_HANDLE;
    VkPhysicalDevice physical        = VK_NULL_HANDLE;
    uint32_t         queueFamilyIdx  = 0;
    VkDevice         device          = VK_NULL_HANDLE;
    VkQueue          queue           = VK_NULL_HANDLE;
    VkSurfaceKHR     surface         = VK_NULL_HANDLE;
    VkSwapchainKHR   swapchain       = VK_NULL_HANDLE;
    VkFormat         swapFormat      = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D       extent{};
    std::vector<VkImage>      images;
    std::vector<VkImageView>  imageViews;
    VkRenderPass     renderPass     = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool    cmdPool        = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> cmdBuffers;
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence>     inFlightFences;

    // Geometry
    VkBuffer         vertexBuffer   = VK_NULL_HANDLE;
    VkDeviceMemory   vertexMemory   = VK_NULL_HANDLE;
    VkBuffer         indexBuffer    = VK_NULL_HANDLE;
    VkDeviceMemory   indexMemory    = VK_NULL_HANDLE;
    uint32_t         indexCount     = 0;

    // Instance data buffer (MVP rows + color per instance)
    VkBuffer         instanceBuffer = VK_NULL_HANDLE;
    VkDeviceMemory   instanceMemory = VK_NULL_HANDLE;
    VkDeviceSize     instanceBufferSize = 0;

    // Pipeline
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline       pipeline       = VK_NULL_HANDLE;
};

uint32_t findMemoryType(VkPhysicalDevice phys, uint32_t typeFilter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
    for (uint32_t i=0; i<memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    throw std::runtime_error("No suitable memory type");
}

void createBuffer(VulkanContext &ctx,
                  VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
                  VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkBufferCreateInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.size = size;
    bi.usage = usage;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(ctx.device, &bi, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("vkCreateBuffer failed");

    VkMemoryRequirements req{};
    vkGetBufferMemoryRequirements(ctx.device, buffer, &req);
    VkMemoryAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = findMemoryType(ctx.physical, req.memoryTypeBits, props);
    if (vkAllocateMemory(ctx.device, &ai, nullptr, &memory) != VK_SUCCESS)
        throw std::runtime_error("vkAllocateMemory failed");
    vkBindBufferMemory(ctx.device, buffer, memory, 0);
}

std::vector<char> loadFile(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Failed to open " + path);
    f.seekg(0, std::ios::end);
    size_t size = (size_t)f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<char> data(size);
    f.read(data.data(), size);
    return data;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule mod;
    if (vkCreateShaderModule(device, &ci, nullptr, &mod) != VK_SUCCESS)
        throw std::runtime_error("vkCreateShaderModule failed");
    return mod;
}

bool vk_initInstance(VulkanContext &ctx) {
    const char* exts[] = { "VK_KHR_surface", "VK_KHR_wayland_surface" };
    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "spheres_bench_vulkan";
    app.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app;
    ci.enabledExtensionCount = 2;
    ci.ppEnabledExtensionNames = exts;

    VkResult r = vkCreateInstance(&ci, nullptr, &ctx.instance);
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateInstance failed: " << r << "\n";
        return false;
    }
    return true;
}

bool vk_initSurface(VulkanContext &ctx, WaylandApp &wl) {
    PFN_vkCreateWaylandSurfaceKHR fn =
        (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(
            ctx.instance, "vkCreateWaylandSurfaceKHR");
    if (!fn) {
        std::cerr << "[VK] vkCreateWaylandSurfaceKHR not found\n";
        return false;
    }
    VkWaylandSurfaceCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    sci.display = wl.display;
    sci.surface = wl.surface;
    VkResult r = fn(ctx.instance, &sci, nullptr, &ctx.surface);
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateWaylandSurfaceKHR failed: " << r << "\n";
        return false;
    }
    return true;
}

bool vk_pickDeviceAndQueue(VulkanContext &ctx) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(ctx.instance, &count, nullptr);
    if (count == 0) {
        std::cerr << "[VK] No physical devices\n";
        return false;
    }
    std::vector<VkPhysicalDevice> devs(count);
    vkEnumeratePhysicalDevices(ctx.instance, &count, devs.data());
    for (auto pd : devs) {
        uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qCount, nullptr);
        std::vector<VkQueueFamilyProperties> qprops(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qCount, qprops.data());
        for (uint32_t i=0; i<qCount; ++i) {
            VkBool32 support = VK_FALSE;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, ctx.surface, &support) == VK_SUCCESS &&
                support &&
                (qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                ctx.physical = pd;
                ctx.queueFamilyIdx = i;
                return true;
            }
        }
    }
    std::cerr << "[VK] No suitable device/queue\n";
    return false;
}

bool vk_createDevice(VulkanContext &ctx) {
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = ctx.queueFamilyIdx;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;

    const char* devExts[] = { "VK_KHR_swapchain" };
    VkDeviceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &qci;
    ci.enabledExtensionCount = 1;
    ci.ppEnabledExtensionNames = devExts;

    VkResult r = vkCreateDevice(ctx.physical, &ci, nullptr, &ctx.device);
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateDevice failed: " << r << "\n";
        return false;
    }
    vkGetDeviceQueue(ctx.device, ctx.queueFamilyIdx, 0, &ctx.queue);
    return true;
}

bool vk_createSwapchain(VulkanContext &ctx, int width, int height) {
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physical, ctx.surface, &caps);
    uint32_t fmtCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physical, ctx.surface, &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> fmts(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physical, ctx.surface, &fmtCount, fmts.data());
    ctx.swapFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR chosenCS = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	for (auto &f : fmts) {
		if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
			f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			ctx.swapFormat = f.format;
			chosenCS = f.colorSpace;
			break;
		}
	}

	if (ctx.swapFormat == VK_FORMAT_UNDEFINED) {
		// fallback
		ctx.swapFormat = fmts[0].format;
		chosenCS = fmts[0].colorSpace;
	}


    VkExtent2D extent;
    if (caps.currentExtent.width != (uint32_t)-1) {
        extent = caps.currentExtent;
    } else {
        extent.width = (uint32_t)width;
        extent.height = (uint32_t)height;
    }
    ctx.extent = extent;

    uint32_t pmCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx.physical, ctx.surface, &pmCount, nullptr);
    std::vector<VkPresentModeKHR> pmodes(pmCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx.physical, ctx.surface, &pmCount, pmodes.data());
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto pm : pmodes) {
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = pm; break; }
    }

    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = ctx.surface;
    sci.minImageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && sci.minImageCount > caps.maxImageCount)
        sci.minImageCount = caps.maxImageCount;
    sci.imageFormat = ctx.swapFormat;
	sci.imageColorSpace = chosenCS;
    sci.imageExtent = extent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = presentMode;
    sci.clipped = VK_TRUE;

    VkResult r = vkCreateSwapchainKHR(ctx.device, &sci, nullptr, &ctx.swapchain);
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateSwapchainKHR failed: " << r << "\n";
        return false;
    }
    uint32_t imgCount = 0;
    vkGetSwapchainImagesKHR(ctx.device, ctx.swapchain, &imgCount, nullptr);
    ctx.images.resize(imgCount);
    vkGetSwapchainImagesKHR(ctx.device, ctx.swapchain, &imgCount, ctx.images.data());
    return true;
}

bool vk_createImageViews(VulkanContext &ctx) {
    ctx.imageViews.resize(ctx.images.size());
    for (size_t i = 0; i < ctx.images.size(); ++i) {
        VkImageViewCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image = ctx.images[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = ctx.swapFormat;
        ci.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
        };
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.baseMipLevel = 0;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.baseArrayLayer = 0;
        ci.subresourceRange.layerCount = 1;

        if (vkCreateImageView(ctx.device, &ci, nullptr, &ctx.imageViews[i]) != VK_SUCCESS) {
            std::cerr << "[VK] vkCreateImageView failed\n";
            return false;
        }
    }
    return true;
}

bool vk_createRenderPass(VulkanContext &ctx) {
    VkAttachmentDescription color{};
    color.format = ctx.swapFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub{};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &colorRef;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.srcAccessMask = 0;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpci{};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &color;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &sub;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    if (vkCreateRenderPass(ctx.device, &rpci, nullptr, &ctx.renderPass) != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateRenderPass failed\n";
        return false;
    }
    return true;
}

bool vk_createFramebuffers(VulkanContext &ctx) {
    ctx.framebuffers.resize(ctx.imageViews.size());
    for (size_t i = 0; i < ctx.imageViews.size(); ++i) {
        VkImageView atts[] = { ctx.imageViews[i] };
        VkFramebufferCreateInfo fci{};
        fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fci.renderPass = ctx.renderPass;
        fci.attachmentCount = 1;
        fci.pAttachments = atts;
        fci.width = ctx.extent.width;
        fci.height = ctx.extent.height;
        fci.layers = 1;
        if (vkCreateFramebuffer(ctx.device, &fci, nullptr, &ctx.framebuffers[i]) != VK_SUCCESS) {
            std::cerr << "[VK] vkCreateFramebuffer failed\n";
            return false;
        }
    }
    return true;
}

bool vk_createCommandPoolAndBuffers(VulkanContext &ctx) {
    VkCommandPoolCreateInfo pci{};
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.queueFamilyIndex = ctx.queueFamilyIdx;
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(ctx.device, &pci, nullptr, &ctx.cmdPool) != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateCommandPool failed\n";
        return false;
    }
    ctx.cmdBuffers.resize(ctx.images.size());
    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = ctx.cmdPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = (uint32_t)ctx.cmdBuffers.size();
    if (vkAllocateCommandBuffers(ctx.device, &ai, ctx.cmdBuffers.data()) != VK_SUCCESS) {
        std::cerr << "[VK] vkAllocateCommandBuffers failed\n";
        return false;
    }
    return true;
}

bool vk_createSync(VulkanContext &ctx) {
    size_t n = ctx.images.size();
    ctx.imageAvailable.resize(n);
    ctx.renderFinished.resize(n);
    ctx.inFlightFences.resize(n);

    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i=0;i<n;++i) {
        if (vkCreateSemaphore(ctx.device, &sci, nullptr, &ctx.imageAvailable[i]) != VK_SUCCESS ||
            vkCreateSemaphore(ctx.device, &sci, nullptr, &ctx.renderFinished[i]) != VK_SUCCESS ||
            vkCreateFence(ctx.device, &fci, nullptr, &ctx.inFlightFences[i]) != VK_SUCCESS)
        {
            std::cerr << "[VK] create sync objects failed\n";
            return false;
        }
    }
    return true;
}

// Create geometry & instance buffers
void vk_setupGeometry(VulkanContext &ctx, const SphereMesh &sphere, int numObjects) {
    ctx.indexCount = (uint32_t)sphere.indices.size();

    VkDeviceSize vSize = sphere.vertices.size()*sizeof(float);
    VkDeviceSize iSize = sphere.indices.size()*sizeof(unsigned short);

    createBuffer(ctx,
                 vSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 ctx.vertexBuffer, ctx.vertexMemory);

    createBuffer(ctx,
                 iSize,
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 ctx.indexBuffer, ctx.indexMemory);

    void* data = nullptr;
    vkMapMemory(ctx.device, ctx.vertexMemory, 0, vSize, 0, &data);
    std::memcpy(data, sphere.vertices.data(), (size_t)vSize);
    vkUnmapMemory(ctx.device, ctx.vertexMemory);

    vkMapMemory(ctx.device, ctx.indexMemory, 0, iSize, 0, &data);
    std::memcpy(data, sphere.indices.data(), (size_t)iSize);
    vkUnmapMemory(ctx.device, ctx.indexMemory);

    // Instance buffer: each instance has MVP(4x4) + color(vec4) = 20 floats = 80 bytes
    VkDeviceSize perInstance = sizeof(float)*20;
    ctx.instanceBufferSize = perInstance * numObjects;

    createBuffer(ctx,
                 ctx.instanceBufferSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 ctx.instanceBuffer, ctx.instanceMemory);
}

// Create pipeline: no descriptors, MVP+color as per-instance vertex attributes
bool vk_createPipeline(VulkanContext &ctx) {
    auto vertCode = loadFile("sphere_vert.spv");
    auto fragCode = loadFile("sphere_frag.spv");
    VkShaderModule vertModule = createShaderModule(ctx.device, vertCode);
    VkShaderModule fragModule = createShaderModule(ctx.device, fragCode);

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName  = "main";

    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName  = "main";

    // Vertex input bindings
    VkVertexInputBindingDescription bindings[2]{};
    // binding 0: sphere vertex (pos+normal)
    bindings[0].binding   = 0;
    bindings[0].stride    = sizeof(float)*6;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // binding 1: per-instance MVP rows + color
    bindings[1].binding   = 1;
    bindings[1].stride    = sizeof(float)*20;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    // Attributes:
    // location 0: pos (vec3), binding 0, offset 0
    // location 1: normal (vec3), binding 0, offset 12
    // location 2-5: MVP rows (vec4 each), binding 1, offsets 0,16,32,48
    // location 6: color (vec4), binding 1, offset 64
    VkVertexInputAttributeDescription attrs[7]{};
    attrs[0].location = 0; attrs[0].binding = 0; attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[0].offset = 0;
    attrs[1].location = 1; attrs[1].binding = 0; attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[1].offset = sizeof(float)*3;
    for (int i=0;i<4;++i) {
        attrs[2+i].location = 2+i;
        attrs[2+i].binding  = 1;
        attrs[2+i].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
        attrs[2+i].offset   = sizeof(float)*4*i;
    }
    attrs[6].location = 6; attrs[6].binding = 1;
    attrs[6].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    attrs[6].offset   = sizeof(float)*16; // 4 rows = 16 floats -> 64 bytes

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount   = 2;
    vi.pVertexBindingDescriptions      = bindings;
    vi.vertexAttributeDescriptionCount = 7;
    vi.pVertexAttributeDescriptions    = attrs;

    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    ia.primitiveRestartEnable = VK_FALSE;

    VkViewport vp{};
    vp.x = 0.0f; vp.y = 0.0f;
    vp.width  = (float)ctx.extent.width;
    vp.height = (float)ctx.extent.height;
    vp.minDepth = 0.0f; vp.maxDepth = 1.0f;
    VkRect2D sc{};
    sc.offset = {0,0};
    sc.extent = ctx.extent;

    VkPipelineViewportStateCreateInfo vpState{};
    vpState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpState.viewportCount = 1;
    vpState.pViewports    = &vp;
    vpState.scissorCount  = 1;
    vpState.pScissors     = &sc;

    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState cba{};
    cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    cba.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &cba;

    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineLayoutCreateInfo plci{};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if (vkCreatePipelineLayout(ctx.device, &plci, nullptr, &ctx.pipelineLayout) != VK_SUCCESS) {
        std::cerr << "[VK] vkCreatePipelineLayout failed\n";
        vkDestroyShaderModule(ctx.device, vertModule, nullptr);
        vkDestroyShaderModule(ctx.device, fragModule, nullptr);
        return false;
    }

    VkGraphicsPipelineCreateInfo gpci{};
    gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpci.stageCount = 2;
    gpci.pStages    = stages;
    gpci.pVertexInputState   = &vi;
    gpci.pInputAssemblyState = &ia;
    gpci.pViewportState      = &vpState;
    gpci.pRasterizationState = &rs;
    gpci.pMultisampleState   = &ms;
    gpci.pDepthStencilState  = &ds;
    gpci.pColorBlendState    = &cb;
    gpci.layout              = ctx.pipelineLayout;
    gpci.renderPass          = ctx.renderPass;
    gpci.subpass             = 0;

    if (vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &gpci, nullptr, &ctx.pipeline) != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateGraphicsPipelines failed\n";
        vkDestroyPipelineLayout(ctx.device, ctx.pipelineLayout, nullptr);
        vkDestroyShaderModule(ctx.device, vertModule, nullptr);
        vkDestroyShaderModule(ctx.device, fragModule, nullptr);
        return false;
    }

    vkDestroyShaderModule(ctx.device, vertModule, nullptr);
    vkDestroyShaderModule(ctx.device, fragModule, nullptr);
    return true;
}

// Record command buffers (single instanced draw)
bool vk_recordCmdBuffers(VulkanContext &ctx, int instanceCount) {
    for (size_t i=0;i<ctx.cmdBuffers.size();++i) {
        VkCommandBuffer cmd = ctx.cmdBuffers[i];

        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(cmd, &bi) != VK_SUCCESS) {
            std::cerr << "[VK] vkBeginCommandBuffer failed\n";
            return false;
        }

        VkClearValue clearColor{};
        clearColor.color = { {0.05f, 0.05f, 0.07f, 1.0f} };

        VkRenderPassBeginInfo rpBegin{};
        rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.renderPass = ctx.renderPass;
        rpBegin.framebuffer = ctx.framebuffers[i];
        rpBegin.renderArea.offset = {0,0};
        rpBegin.renderArea.extent = ctx.extent;
        rpBegin.clearValueCount = 1;
        rpBegin.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline);

        VkBuffer bufs[2] = { ctx.vertexBuffer, ctx.instanceBuffer };
        VkDeviceSize offs[2] = { 0, 0 };
        vkCmdBindVertexBuffers(cmd, 0, 2, bufs, offs);
        vkCmdBindIndexBuffer(cmd, ctx.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(cmd, ctx.indexCount, instanceCount, 0, 0, 0);

        vkCmdEndRenderPass(cmd);
        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            std::cerr << "[VK] vkEndCommandBuffer failed\n";
            return false;
        }
    }
    return true;
}

void vk_destroy(VulkanContext &ctx) {
    if (ctx.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(ctx.device);

        if (ctx.pipeline)       vkDestroyPipeline(ctx.device, ctx.pipeline, nullptr);
        if (ctx.pipelineLayout) vkDestroyPipelineLayout(ctx.device, ctx.pipelineLayout, nullptr);

        if (ctx.instanceBuffer) vkDestroyBuffer(ctx.device, ctx.instanceBuffer, nullptr);
        if (ctx.instanceMemory) vkFreeMemory(ctx.device, ctx.instanceMemory, nullptr);

        if (ctx.indexBuffer) vkDestroyBuffer(ctx.device, ctx.indexBuffer, nullptr);
        if (ctx.indexMemory) vkFreeMemory(ctx.device, ctx.indexMemory, nullptr);

        if (ctx.vertexBuffer) vkDestroyBuffer(ctx.device, ctx.vertexBuffer, nullptr);
        if (ctx.vertexMemory) vkFreeMemory(ctx.device, ctx.vertexMemory, nullptr);

        for (auto f: ctx.inFlightFences)  if (f) vkDestroyFence(ctx.device, f, nullptr);
        for (auto s: ctx.renderFinished)  if (s) vkDestroySemaphore(ctx.device, s, nullptr);
        for (auto s: ctx.imageAvailable)  if (s) vkDestroySemaphore(ctx.device, s, nullptr);
        if (!ctx.cmdBuffers.empty() && ctx.cmdPool)
            vkFreeCommandBuffers(ctx.device, ctx.cmdPool,
                                 (uint32_t)ctx.cmdBuffers.size(), ctx.cmdBuffers.data());
        if (ctx.cmdPool) vkDestroyCommandPool(ctx.device, ctx.cmdPool, nullptr);
        for (auto fb: ctx.framebuffers) if (fb) vkDestroyFramebuffer(ctx.device, fb, nullptr);
        if (ctx.renderPass) vkDestroyRenderPass(ctx.device, ctx.renderPass, nullptr);
        for (auto iv: ctx.imageViews) if (iv) vkDestroyImageView(ctx.device, iv, nullptr);
        if (ctx.swapchain) vkDestroySwapchainKHR(ctx.device, ctx.swapchain, nullptr);
        vkDestroyDevice(ctx.device, nullptr);
    }
    if (ctx.surface && ctx.instance) vkDestroySurfaceKHR(ctx.instance, ctx.surface, nullptr);
    if (ctx.instance) vkDestroyInstance(ctx.instance, nullptr);
}

void runVulkanSpheres(const Args &args) {
    WaylandApp wl{};
    if (!initWayland(wl)) return;
	
    VulkanContext ctx{};
    if (!vk_initInstance(ctx)) return;
	if (!vk_initSurface(ctx, wl)) { vk_destroy(ctx); return; }
    if (!vk_pickDeviceAndQueue(ctx)) { vk_destroy(ctx); return; }
    if (!vk_createDevice(ctx)) { vk_destroy(ctx); return; }
    if (!vk_createSwapchain(ctx, args.width, args.height)) { vk_destroy(ctx); return; }
    if (!vk_createImageViews(ctx)) { vk_destroy(ctx); return; }
    if (!vk_createRenderPass(ctx)) { vk_destroy(ctx); return; }
    if (!vk_createFramebuffers(ctx)) { vk_destroy(ctx); return; }
    if (!vk_createCommandPoolAndBuffers(ctx)) { vk_destroy(ctx); return; }
    if (!vk_createSync(ctx)) { vk_destroy(ctx); return; }

    SphereMesh sphere = makeQualitySphere(args.quality);
    std::cout << "[VK] Sphere vertices: " << sphere.vertices.size()/6
              << " indices: " << sphere.indices.size() << "\n";

    std::vector<Object> objects = createObjects(args.numObjects);
    std::cout << "[VK] Objects: " << objects.size() << "\n";

    try {
        vk_setupGeometry(ctx, sphere, args.numObjects);
        if (!vk_createPipeline(ctx)) {
            vk_destroy(ctx); return;
        }
        if (!vk_recordCmdBuffers(ctx, args.numObjects)) {
            vk_destroy(ctx); return;
        }
    } catch (const std::exception &e) {
        std::cerr << "[VK] Exception: " << e.what() << "\n";
        vk_destroy(ctx);
        return;
    }

    Mat4 proj = matPerspective(45.0f * (3.1415926f/180.0f),
                               (float)args.width / (float)args.height,
                               0.1f, 100.0f);

    getCPUUsage();
    double cpuStart = getCPUUsage();
    auto start = std::chrono::high_resolution_clock::now();
    auto lastReportTime = std::chrono::high_resolution_clock::now();
    int framesThisSecond = 0;
    double lastCPU = getCPUUsage();

    bool overlayReady = false;
	
    // main loop
    for (int frame=0; frame<args.frames; ++frame) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        // Update instance buffer: per-object MVP and color
        float timeSec = (float)std::chrono::duration<double>(
            frameStart - start).count();
        VkDeviceSize perInstance = sizeof(float)*20;
        void* data = nullptr;
        vkMapMemory(ctx.device, ctx.instanceMemory, 0, ctx.instanceBufferSize, 0, &data);
        float *dst = (float*)data;
        for (int i=0;i<args.numObjects;++i) {
            Object &o = objects[i];
            doCPUWork(args, o, frame);
            float angle = o.baseAngle + o.rotationSpeed * timeSec;
            Mat4 model  = matMul(matTranslate(o.position), matRotateY(angle));
            Mat4 mvp    = matMul(proj, model);
            // write MVP rows (column-major -> treat as is)
            for (int j=0;j<16;++j) dst[j] = mvp.m[j];
            dst[16] = o.color.x;
            dst[17] = o.color.y;
            dst[18] = o.color.z;
            dst[19] = 1.0f;
            dst += 20;
        }
        vkUnmapMemory(ctx.device, ctx.instanceMemory);

        uint32_t imgIndex = 0;
        VkResult r = vkAcquireNextImageKHR(
            ctx.device, ctx.swapchain, UINT64_MAX,
            ctx.imageAvailable[frame % ctx.images.size()],
            VK_NULL_HANDLE, &imgIndex);
        if (r != VK_SUCCESS) {
            std::cerr << "[VK] vkAcquireNextImageKHR failed: " << r << "\n";
            break;
        }

        VkFence fence = ctx.inFlightFences[imgIndex];
        vkWaitForFences(ctx.device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkResetFences(ctx.device, 1, &fence);

        VkSemaphore waitSem[] = { ctx.imageAvailable[frame % ctx.images.size()] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSem[] = { ctx.renderFinished[frame % ctx.images.size()] };

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount   = 1;
        submit.pWaitSemaphores      = waitSem;
        submit.pWaitDstStageMask    = waitStages;
        submit.commandBufferCount   = 1;
        submit.pCommandBuffers      = &ctx.cmdBuffers[imgIndex];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores    = signalSem;

        r = vkQueueSubmit(ctx.queue, 1, &submit, fence);
        if (r != VK_SUCCESS) {
            std::cerr << "[VK] vkQueueSubmit failed: " << r << "\n";
            break;
        }

        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = signalSem;
        present.swapchainCount = 1;
        present.pSwapchains = &ctx.swapchain;
        present.pImageIndices = &imgIndex;

        r = vkQueuePresentKHR(ctx.queue, &present);
        if (r != VK_SUCCESS) {
            std::cerr << "[VK] vkQueuePresentKHR failed: " << r << "\n";
            break;
        }
		
		if (!overlayReady) {
			overlayInit(
			    wl.display,
			    wl.compositor,
			    wl.shell,
			    EGL_NO_DISPLAY, 
				EGL_NO_CONTEXT,
			    args.width,
			    args.height
			);
			overlayShowInfo(
				"MOTION_BENCH",
				"VULKAN",
				args.numObjects
			);
			overlayReady = true;
		}
        
	framesThisSecond++;

	auto now = std::chrono::high_resolution_clock::now();
	double elapsed =
	    std::chrono::duration<double>(now - lastReportTime).count();

	if (elapsed >= 1.0) {
	    double currentCPU = getCPUUsage();

	    double fpsThisSecond = framesThisSecond / elapsed;
	    double cpuDelta = currentCPU - lastCPU;

	    std::cout << "[VK] FPS: " << fpsThisSecond
	              << " CPU: " << currentCPU << "% "
	              << "(Δ " << cpuDelta << "%)"
	              << std::endl;
		overlayUpdatePerf(fpsThisSecond, currentCPU);
	    appendCSV(args.csvFile, "Vulkan", args.numObjects, args.quality, args.cpuWork,
              framesThisSecond, currentCPU);
	    // reset for next interval
	    framesThisSecond = 0;
	    lastReportTime = now;
	    lastCPU = currentCPU;
	}
        capFrameTime(args, frameStart);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double cpuEnd = getCPUUsage();
    double secs  = std::chrono::duration<double>(end - start).count();
    double fps   = args.frames / (secs > 0.0 ? secs : 1.0);

    std::cout << "[VK] FPS: " << fps
              << "% CPU End: " << cpuEnd << "%\n";
    appendCSV(args.csvFile, "Vulkan", args.numObjects, args.quality, args.cpuWork,
              fps, cpuEnd);

    overlayShutdown();
    vk_destroy(ctx);

	
    if (wl.surface)    wl_proxy_destroy((wl_proxy*)wl.surface);
    if (wl.compositor) wl_proxy_destroy((wl_proxy*)wl.compositor);
    if (wl.registry)   wl_proxy_destroy((wl_proxy*)wl.registry);
    if (wl.display)    wl_display_disconnect(wl.display);
}

// ----------------- main -----------------

int main(int argc, char** argv) {
    Args args = parseArgs(argc, argv);
    std::cout << "spheres_bench: API="
              << (args.api == Args::API::GL ? "OpenGL" : "Vulkan")
              << " frames=" << args.frames
              << " size="   << args.width << "x" << args.height
              << " objects="<< args.numObjects
              << " quality="<< args.quality
              << " cpuWork="<< args.cpuWork
              << " targetFPS="<< args.targetFPS
              << "\n";

    if (args.api == Args::API::GL)
        runGLSpheres(args);
    else
        runVulkanSpheres(args);

    return 0;
}

/*
===========================================================
 Suggested GLSL for Vulkan (compile to SPIR-V offline)
===========================================================

-- sphere.vert --
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;

// MVP matrix rows as instance attributes:
layout(location=2) in vec4 inRow0;
layout(location=3) in vec4 inRow1;
layout(location=4) in vec4 inRow2;
layout(location=5) in vec4 inRow3;
layout(location=6) in vec4 inColor;

layout(location=0) out vec3 vNormal;
layout(location=1) out vec3 vColor;

void main() {
    mat4 mvp = mat4(inRow0, inRow1, inRow2, inRow3);
    vColor = inColor.rgb;
    vNormal = normalize(inNormal);
    gl_Position = mvp * vec4(inPos, 1.0);
}

Compile:
  glslangValidator -V sphere.vert -o sphere_vert.spv

-- sphere.frag --
#version 450
layout(location=0) in vec3 vNormal;
layout(location=1) in vec3 vColor;
layout(location=0) out vec4 outColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(0.5, 1.0, 0.2));
    float diff = max(dot(N, L), 0.0);
    vec3 color = vColor * (0.2 + 0.8 * diff);
    outColor = vec4(color, 1.0);
}

Compile:
  glslangValidator -V sphere.frag -o sphere_frag.spv
*/
