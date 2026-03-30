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

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "overlay.h"
#include <simpleshell-client-protocol.h>

// --------------------------------------------------
// Defaults (overridable via CLI)
// --------------------------------------------------
int WIDTH  = 1920;
int HEIGHT = 1080;
int THREADS = 4;
int RECTS = 5000;
double RUN_TIME = 30.0;
bool overlay = true;
std::string csvfile = "multithread_test.csv";

// --------------------------------------------------
// CLI
// --------------------------------------------------
void parseArgs(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--width") && i + 1 < argc) 
            WIDTH = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--height") && i + 1 < argc) 
            HEIGHT = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--threads") && i + 1 < argc) 
            THREADS = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--rects") && i + 1 < argc)
            RECTS = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--time") && i + 1 < argc)
            RUN_TIME = atof(argv[++i]);
        else if (!strcmp(argv[i], "--no-overlay"))
            overlay = false;
        else if (!strcmp(argv[i], "--csv") && i + 1 < argc)
            csvfile = argv[++i];
    }
}

void appendCSV(double fps, double cpuEnd)
{
    bool exists = access(csvfile.c_str(), F_OK) == 0;
    std::string api = "OpenGL";
    std::ofstream out(csvfile, std::ios::app);
    if (!exists) out << "API,Resolution,Rectangles,FPS,CPU_Usage\n";
    out << api << "," << WIDTH << "x" << HEIGHT << "," << RECTS << "," << fps << "," << cpuEnd << "\n";
}

// --------------------------------------------------
// CPU usage
// --------------------------------------------------
static unsigned long long prevTotal = 0, prevIdle = 0;

double getCPUUsage()
{
    std::ifstream f("/proc/stat");
    if (!f.is_open()) return 0.0;

    std::string cpu;
    unsigned long long u,n,s,i,io,irq,si,st;
    f >> cpu >> u >> n >> s >> i >> io >> irq >> si >> st;

    unsigned long long idleAll = i + io;
    unsigned long long nonIdle = u + n + s + irq + si + st;
    unsigned long long total = idleAll + nonIdle;

    double pct = 0.0;
    if (prevTotal) {
        unsigned long long td = total - prevTotal;
        unsigned long long id = idleAll - prevIdle;
        if (td) pct = (double)(td - id) / td * 100.0;
    }
    prevTotal = total;
    prevIdle = idleAll;
    return pct;
}

// --------------------------------------------------
// Wayland
// --------------------------------------------------
struct WL {
    wl_display* d{};
    wl_registry* r{};
    wl_compositor* c{};
    wl_surface* s{};
    wl_egl_window* egl{};
	wl_simple_shell* shell{};
};

static void reg_add(void* data, wl_registry* r,
                    uint32_t id, const char* iface, uint32_t)
{
    if (!strcmp(iface, "wl_compositor")) {
        ((WL*)data)->c =
            (wl_compositor*)wl_registry_bind(
                r, id, &wl_compositor_interface, 1);
    }
	else if (!strcmp(iface, "wl_simple_shell"))
        ((WL*)data)->shell = (wl_simple_shell*)
            wl_registry_bind(r, id, &wl_simple_shell_interface, 1);
}
static void reg_rem(void*, wl_registry*, uint32_t) {}

bool initWayland(WL& wl)
{
    wl.d = wl_display_connect(nullptr);
    if (!wl.d) return false;

    wl.r = wl_display_get_registry(wl.d);
    static wl_registry_listener l{reg_add, reg_rem};
    wl_registry_add_listener(wl.r, &l, &wl);
    wl_display_roundtrip(wl.d);

    wl.s = wl_compositor_create_surface(wl.c);
    if (!wl.s) return false;

    wl.egl = wl_egl_window_create(wl.s, WIDTH, HEIGHT);
    if (!wl.egl) return false;

    return true;
}

// --------------------------------------------------
// Rect
// --------------------------------------------------
struct Rect {
    float x, y;
    float r, g, b;
};

// --------------------------------------------------
// Shader helpers
// --------------------------------------------------
GLuint compile(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    
    GLint success;
    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(s, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
        glDeleteShader(s);
        return 0;
    }
    
    return s;
}

// --------------------------------------------------
// Main
// --------------------------------------------------
int main(int argc, char** argv)
{
    parseArgs(argc, argv);

    std::cout
        << "[CONFIG] "
        << "size=" << WIDTH << "x" << HEIGHT
        << " threads=" << THREADS
        << " rects=" << RECTS
        << " time=" << RUN_TIME << "s\n";

    // ---------------- Wayland ----------------
    WL wl{};
    if (!initWayland(wl)) {
        std::cerr << "Wayland init failed\n";
        return -1;
    }

    // ---------------- EGL ----------------
    EGLDisplay edpy = eglGetDisplay((EGLNativeDisplayType)wl.d);
    if (edpy == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display\n";
        return -1;
    }
    
    if (!eglInitialize(edpy, nullptr, nullptr)) {
        std::cerr << "Failed to initialize EGL\n";
        return -1;
    }

    EGLint cfgAttr[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };

    EGLConfig cfg;
    EGLint num;
    if (!eglChooseConfig(edpy, cfgAttr, &cfg, 1, &num) || num == 0) {
        std::cerr << "Failed to choose EGL config\n";
        return -1;
    }

    EGLSurface surf = eglCreateWindowSurface(
        edpy, cfg,
        (EGLNativeWindowType)wl.egl, nullptr);
    if (surf == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL surface\n";
        return -1;
    }

    EGLint ctxAttr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext ctx = eglCreateContext(
        edpy, cfg, EGL_NO_CONTEXT, ctxAttr);
    if (ctx == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL context\n";
        return -1;
    }

    if (!eglMakeCurrent(edpy, surf, surf, ctx)) {
        std::cerr << "Failed to make EGL context current\n";
        return -1;
    }

    // ---------------- Shaders ----------------
    const char* vs = R"(
        attribute vec2 aPos;
        uniform vec2 uPos;
        void main() {
            gl_Position = vec4(aPos + uPos, 0.0, 1.0);
        }
    )";

    const char* fs = R"(
        precision mediump float;
        uniform vec3 uColor;
        void main() {
            gl_FragColor = vec4(uColor, 1.0);
        }
    )";

    GLuint prog = glCreateProgram();
    GLuint vertShader = compile(GL_VERTEX_SHADER, vs);
    GLuint fragShader = compile(GL_FRAGMENT_SHADER, fs);
    
    if (vertShader == 0 || fragShader == 0) {
        std::cerr << "Shader compilation failed\n";
        return -1;
    }
    
    glAttachShader(prog, vertShader);
    glAttachShader(prog, fragShader);
    glLinkProgram(prog);
    
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(prog, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
        return -1;
    }
    
    // Clean up shader objects
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    glUseProgram(prog);

    GLint uPos   = glGetUniformLocation(prog, "uPos");
    GLint uColor = glGetUniformLocation(prog, "uColor");

    GLfloat quad[] = {
        -0.02f,-0.02f,  0.02f,-0.02f,  0.02f, 0.02f,
        -0.02f,-0.02f,  0.02f, 0.02f, -0.02f, 0.02f
    };

    GLint aPos = glGetAttribLocation(prog, "aPos");
    glVertexAttribPointer(aPos, 2, GL_FLOAT, GL_FALSE, 0, quad);
    glEnableVertexAttribArray(aPos);

    glViewport(0, 0, WIDTH, HEIGHT);

    // ---------------- Rects ----------------
    std::vector<Rect> rects(RECTS);
    for (auto& r : rects) {
        r.x = (rand() / float(RAND_MAX) - 0.5f) * 2.0f;
        r.y = (rand() / float(RAND_MAX) - 0.5f) * 2.0f;
        r.r = rand() / float(RAND_MAX);
        r.g = rand() / float(RAND_MAX);
        r.b = rand() / float(RAND_MAX);
    }

    auto start = std::chrono::steady_clock::now();
    auto lastStat = start;
    int frames = 0;
    getCPUUsage();
    
    // Variables for average calculation
    double totalFPS = 0.0;
    double totalCPU = 0.0;
    int sampleCount = 0;
    
    bool overlayReady = false;

    // ---------------- Render loop ----------------
    while (true) {
        auto now = std::chrono::steady_clock::now();
        double t = std::chrono::duration<double>(now - start).count();
        if (t > RUN_TIME) break;

        // ---- CPU multithreaded update (NO GL) ----
        std::vector<std::thread> workers;
        for (int th = 0; th < THREADS; th++) {
            workers.emplace_back([&, th] {
                for (int i = th; i < RECTS; i += THREADS) {
                    rects[i].x += sin(t + i) * 0.0005f;
                }
            });
        }
        for (auto& w : workers) w.join();

        // ---- Render thread (GL ONLY) ----
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& r : rects) {
            glUniform2f(uPos, r.x, r.y);
            glUniform3f(uColor, r.r, r.g, r.b);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        eglSwapBuffers(edpy, surf);
		
        if (overlay && !overlayReady) {
            overlayInit(
                wl.d,
                wl.c,
                wl.shell,
                edpy,
                ctx,
                WIDTH,
                HEIGHT
            );
            overlayShowInfo("MULTI THREADING TEST","OPENGL",RECTS);
            overlayReady = true;
        }

        wl_display_dispatch_pending(wl.d);
        wl_display_flush(wl.d);

        // ---- Stats ----
        frames++;
        double sec =
            std::chrono::duration<double>(now - lastStat).count();
        if (sec >= 1.0) {
            double fps = frames/sec;
            double cpu = getCPUUsage();
            
            // Accumulate for average calculation
            totalFPS += fps;
            totalCPU += cpu;
            sampleCount++;
            
            std::cout << "[FPS] " << fps
                      << "  CPU " << cpu << "%\n";
            if (overlay) {
                overlayUpdatePerf(fps, cpu);
            }
            appendCSV(fps,cpu);
            frames = 0;
            lastStat = now;
        }
    }

    // Calculate and display final statistics
    auto end = std::chrono::steady_clock::now();
    double totalRuntime = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== Test Results ===\n";
    std::cout << "Total Runtime: " << totalRuntime << " seconds\n";
    std::cout << "Threads: " << THREADS << "\n";
    std::cout << "Rectangles: " << RECTS << "\n";
    std::cout << "Resolution: " << WIDTH << "x" << HEIGHT << "\n";
    
    if (sampleCount > 0) {
        double avgFPS = totalFPS / sampleCount;
        double avgCPU = totalCPU / sampleCount;
        std::cout << "Average FPS: " << avgFPS << "\n";
        std::cout << "Average CPU Usage: " << avgCPU << "%\n";
        
        // Export final averages to CSV
        appendCSV(avgFPS, avgCPU);
    }

    if (overlay) {
        overlayShutdown();
    }
	
    // ---------------- Cleanup ----------------
    eglMakeCurrent(edpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(edpy, ctx);
    eglDestroySurface(edpy, surf);
    eglTerminate(edpy);

    wl_egl_window_destroy(wl.egl);
    wl_surface_destroy(wl.s);
    wl_display_disconnect(wl.d);

    std::cout << "Exited cleanly\n";
    return 0;
}
