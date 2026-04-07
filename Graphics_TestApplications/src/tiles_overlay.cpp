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

#define VK_USE_PLATFORM_WAYLAND_KHR

#include <vulkan/vulkan.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <simpleshell-client-protocol.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <cstring>
#include <thread>
#include <unistd.h>
#include "overlay.h"

struct Args {
    enum class API { GL, Vulkan };

    API api = API::GL;
    int width  = 1920;
    int height = 1080;
    int tilesX = 10;
    int tilesY = 10;
    int fpsCap = 60;
    bool vsync = true;
    std::string csvFile = "tiles.csv";
};

static bool overlay = true;
static int runtime = 30;

Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--api=gl") == 0) a.api = Args::API::GL;
        else if (strcmp(argv[i], "--api=vulkan") == 0) a.api = Args::API::Vulkan;
        else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) { a.width = std::atoi(argv[++i]); }
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) { a.height = std::atoi(argv[++i]); }
        else if (strcmp(argv[i], "--tiles-x") == 0 && i + 1 < argc) { a.tilesX = std::atoi(argv[++i]); }
        else if (strcmp(argv[i], "--tiles-y") == 0 && i + 1 < argc) { a.tilesY = std::atoi(argv[++i]); }
        else if (strcmp(argv[i], "--csv") == 0 && i + 1 < argc) a.csvFile = argv[++i];
        else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) a.fpsCap = std::atoi(argv[++i]);
        else if (strcmp(argv[i], "--vsync") == 0 && i + 1 < argc) a.vsync = std::atoi(argv[++i]);
        else if (strcmp(argv[i], "--runtime") == 0 && i + 1 < argc) runtime = std::atoi(argv[++i]);
    }
    return a;
}

void appendCSV(const std::string &file, const std::string &api,
               int width, int height, int tilesX, int tilesY, double fps,
               double cpuEnd)
{
    bool exists = access(file.c_str(), F_OK) == 0;
    std::ofstream out(file, std::ios::app);
    if (!exists) out << "API,Resolution,TilesX,TilesY,FPS,CPU_Usage\n";
    out << api << "," << width << "x" << height << "," << tilesX << "," << tilesY << ","
        << fps << "," << cpuEnd << "\n";
}

inline void capFrameTime(int fps,
                         const std::chrono::high_resolution_clock::time_point &frameStart)
{
    if (fps <= 0) return; // no cap

    using namespace std::chrono;
    double targetSeconds = 1.0 / (double)fps;
    auto now = high_resolution_clock::now();
    double elapsed = duration<double>(now - frameStart).count();
    double remaining = targetSeconds - elapsed;
    if (remaining > 0.0) {
        std::this_thread::sleep_for(duration<double>(remaining));
    }
}

/* =========================================================
 * CPU usage helper
 * ========================================================= */
static unsigned long long pTot=0,pIdle=0;
static double getCPUUsage()
{
    std::ifstream f("/proc/stat");
    std::string cpu;
    unsigned long long u,n,s,i,iw,ir,si,st;
    f>>cpu>>u>>n>>s>>i>>iw>>ir>>si>>st;
    unsigned long long idle=i+iw;
    unsigned long long tot=idle+u+n+s+ir+si+st;
    double r=0;
    if(pTot){
        unsigned long long dt=tot-pTot, di=idle-pIdle;
        r=(double)(dt-di)/dt*100.0;
    }
    pTot=tot; pIdle=idle;
    return r;
}

/* =========================================================
 * Wayland app (tiles owns globals)
 * ========================================================= */
struct WaylandApp {
    wl_display* display=nullptr;
    wl_registry* registry=nullptr;
    wl_compositor* compositor=nullptr;
    wl_simple_shell* shell=nullptr;
    wl_surface* surface=nullptr;
    wl_egl_window* eglWin=nullptr;
};

static void reg_cb(void* d, wl_registry* r,
                   uint32_t id,const char* iface,uint32_t)
{
    WaylandApp* a=(WaylandApp*)d;
    if(!strcmp(iface,"wl_compositor"))
        a->compositor=(wl_compositor*)wl_registry_bind(r,id,&wl_compositor_interface,1);
    else if(!strcmp(iface,"wl_simple_shell"))
        a->shell=(wl_simple_shell*)wl_registry_bind(r,id,&wl_simple_shell_interface,1);
}

static const wl_registry_listener regListener={reg_cb,nullptr};

static bool initWayland(WaylandApp& a)
{
    a.display=wl_display_connect(nullptr);
    a.registry=wl_display_get_registry(a.display);
    wl_registry_add_listener(a.registry,&regListener,&a);
    wl_display_roundtrip(a.display);
    if(!a.compositor||!a.shell) return false;
    a.surface=wl_compositor_create_surface(a.compositor);
    return true;
}

/* =========================================================
 * Tiles renderer
 * ========================================================= */
static void runGLTiles(const Args& args)
{
    WaylandApp app{};
    if(!initWayland(app)) return;
    
    int tileW = args.width / args.tilesX;
    int tileH = args.height / args.tilesY;

    app.eglWin = wl_egl_window_create(app.surface,args.width,args.height);
    if (!app.eglWin) {
        std::cerr << "[GL] Failed to create EGL window\n";
        return;
    }
    
    EGLDisplay ed=eglGetDisplay((EGLNativeDisplayType)app.display);
    if (ed == EGL_NO_DISPLAY) {
        std::cerr << "[GL] Failed to get EGL display\n";
        return;
    }
    
    if (!eglInitialize(ed,nullptr,nullptr)) {
        std::cerr << "[GL] Failed to initialize EGL\n";
        return;
    }

    EGLint cfgA[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,
                   EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_NONE};
    EGLConfig cfg; EGLint n;
    eglChooseConfig(ed,cfgA,&cfg,1,&n);
    EGLint ctxA[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};
    EGLContext ctx=eglCreateContext(ed,cfg,EGL_NO_CONTEXT,ctxA);
    EGLSurface surf=eglCreateWindowSurface(
        ed,cfg,(EGLNativeWindowType)app.eglWin,nullptr);
    eglMakeCurrent(ed,surf,surf,ctx);
    /* ENABLE VSYNC */
    EGLBoolean ok = eglSwapInterval(ed, args.vsync ? 1 : 0);
    std::cout << "VSync enabled: " << (args.vsync ? "true" : "false") << " (result: " << ok << ")" << std::endl;

    glEnable(GL_SCISSOR_TEST);
    getCPUUsage();

    

    /* ---- latency & stats ---- */
    double latencySumMs = 0.0;
    double maxLatencyMs = 0.0;
    int    measuredFrames = 0;

    const int WARMUP_FRAMES = 1;   // skip first frame (same as Vulkan)
    const double WARMUP_TIME  = 1.0;
    bool latencyActive = false;
    int frames = 0;

    /* ---- PRIME CPU (fix nan / spike) ---- */
    getCPUUsage();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    getCPUUsage();

    auto t0=std::chrono::high_resolution_clock::now();
    auto last=t0;
    double cpuSum=0; int cpuN=0;
    int frame = 0;
    bool overlayReady = false;
    while (true)
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        glViewport(0, 0, args.width, args.height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int ty = 0; ty < args.tilesY; ++ty) {
            for (int tx = 0; tx < args.tilesX; ++tx) {
                int x = tx * tileW;
                int y = ty * tileH;
    
                int w = (tx == args.tilesX - 1) ? (args.width  - x) : tileW;
                int h = (ty == args.tilesY - 1) ? (args.height - y) : tileH;

                glScissor(x, y, w, h);

                float r = float(tx) / args.tilesX;
                float g = float(ty) / args.tilesY;
                float b = float(frame % 256) / 255.0f;

                glClearColor(r, g, b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
        }

        eglSwapBuffers(ed, surf);

        /* ---- GPU COMPLETE (critical) ---- */
        glFinish();

        if (overlay && !overlayReady)
        {
            overlayInit(app.display,
                        app.compositor,
                        app.shell,
                        ed,
                        ctx,
                        args.width,
                        args.height);

            overlayShowInfo("TILES BENCH","OPENGL", args.tilesX);
            overlayReady = true;
        }

        auto frameEnd = std::chrono::high_resolution_clock::now();

        /* ---- latency (skip warmup) ---- */
        double sinceStart = std::chrono::duration<double>(frameEnd - t0).count();

        /* Enable latency measurement only after warmup */
        if (!latencyActive &&
                frame >= WARMUP_FRAMES &&
                sinceStart >= WARMUP_TIME)
        {
            latencyActive = true;

            /* RESET worst latency after warmup */
            latencySumMs  = 0.0;
            maxLatencyMs  = 0.0;
            measuredFrames = 0;

            std::cout << "[GL] Latency measurement started (warmup complete)\n";
        }

        if (latencyActive) {
            double frameLatencyMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();

            latencySumMs += frameLatencyMs;
            if ((maxLatencyMs > 100) && (!maxLatencyMs))
            {
                std::cout<<" [GL] Ignoring first maxLatencyMs\n";
                maxLatencyMs  = 0.0;
            }
            if (frameLatencyMs > maxLatencyMs)
                maxLatencyMs = frameLatencyMs;

            measuredFrames++;
        }


        frames++;

        auto now = std::chrono::high_resolution_clock::now();
        double elapsed =
            std::chrono::duration<double>(now - last).count();
    
        if (elapsed >= 1.0) {
            double cpu = getCPUUsage();
            double fps = frames / elapsed;
            double avgLatency =
                measuredFrames ? (latencySumMs / measuredFrames) : 0.0;

            std::cout
                << "[GL] FPS: " << fps
                << " | CPU: " << cpu << "%"
                << " | Avg Latency: " << avgLatency << " ms"
                << " | Max Latency: " << maxLatencyMs << " ms\n";

            if (overlay)
                overlayUpdatePerf(fps, cpu);

            appendCSV(args.csvFile, "OpenGL",
                      args.width, args.height, args.tilesX, args.tilesY,
                      fps, cpu);

            cpuSum += cpu;
            cpuN++;

            frames = 0;
            last = now;
        }

        capFrameTime(args.fpsCap, frameStart);
        
        auto t1 = std::chrono::high_resolution_clock::now();
        double totalRuntimeSec = std::chrono::duration<double>(t1 - t0).count();
        if (totalRuntimeSec > runtime)
        {
            std::cout << "Exiting App as " << runtime << " seconds is reached \n";
            break;
        }
        frame++;        
    }

    /* ===================== FINAL STATISTICS ===================== */

    auto t1 = std::chrono::high_resolution_clock::now();
    double totalRuntimeSec =
        std::chrono::duration<double>(t1 - t0).count();

    /* ---- Average FPS (exclude warmup frames) ---- */
    int effectiveFrames = frame - WARMUP_FRAMES;
    if (effectiveFrames < 1) effectiveFrames = 1;

    double avgFPS = effectiveFrames / totalRuntimeSec;
    
    /* ---- Average CPU ---- */
    double avgCPU = (cpuN > 0) ? (cpuSum / cpuN) : 0.0;

    /* ---- Average latency ---- */
    double avgLatencyMs =
        (measuredFrames > 0) ? (latencySumMs / measuredFrames) : 0.0;

    /* ---- Print (UNIFORM with Vulkan) ---- */
    std::cout << "[GL] Average FPS " << avgFPS << "\n";
    std::cout << "[GL] Average CPU " << avgCPU << "\n";
    std::cout << "[GL] Average Frame Latency: "
              << avgLatencyMs << " ms\n";
    std::cout << "[GL] Worst Frame Latency: "
              << maxLatencyMs << " ms\n";
    std::cout << "[GL] Total Frames Rendered :" 
              << frame << "\n";

    /* ---- CSV (summary row) ---- */
    appendCSV(args.csvFile,
              "OpenGL",
              args.width,
              args.height,
              args.tilesX,
              args.tilesY,
              avgFPS,
              avgCPU);

    if (overlay)
        overlayShutdown();
    wl_display_roundtrip(app.display);

    eglMakeCurrent(ed,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    eglDestroySurface(ed,surf);
    eglDestroyContext(ed,ctx);
    eglTerminate(ed);
    wl_egl_window_destroy(app.eglWin);
    wl_surface_destroy(app.surface);
    wl_display_disconnect(app.display);
}

// -------------------- Vulkan tiles path (no shaders, clear attachments) --------------------

struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> cmdBuffers;
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence> inFlightFences;
};

bool initVulkanInstance(VulkanContext &ctx) {
    std::vector<const char*> exts = {
        "VK_KHR_surface",
        "VK_KHR_wayland_surface"
    };

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "tiles_bench_vulkan";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;
    ci.enabledExtensionCount = (uint32_t)exts.size();
    ci.ppEnabledExtensionNames = exts.data();

    VkResult r = vkCreateInstance(&ci, nullptr, &ctx.instance);
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateInstance failed: " << r << "\n";
        return false;
    }
    std::cout << "[VK] step: instance created\n";
    return true;
}

bool initVulkanSurface(VulkanContext &ctx, WaylandApp &wl) {
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
    std::cout << "[VK] step: wayland surface created\n";
    return true;
}

bool pickVulkanDeviceAndQueue(VulkanContext &ctx) {
    uint32_t count = 0;
    VkResult r = vkEnumeratePhysicalDevices(ctx.instance, &count, nullptr);
    if (r != VK_SUCCESS || count == 0) {
        std::cerr << "[VK] vkEnumeratePhysicalDevices failed or no devices: " << r << "\n";
        return false;
    }
    std::vector<VkPhysicalDevice> devs(count);
    vkEnumeratePhysicalDevices(ctx.instance, &count, devs.data());

    for (auto pd : devs) {
        uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qCount, nullptr);
        if (qCount == 0) continue;
        std::vector<VkQueueFamilyProperties> qprops(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qCount, qprops.data());
        for (uint32_t i = 0; i < qCount; ++i) {
            VkBool32 support = VK_FALSE;
            VkResult rc = vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, ctx.surface, &support);
            if (rc == VK_SUCCESS && support &&
                (qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                ctx.physical = pd;
                ctx.queueFamilyIndex = i;
                std::cout << "[VK] step: picked physical device + queue family " << i << "\n";
                return true;
            }
        }
    }
    std::cerr << "[VK] No suitable physical device / queue\n";
    return false;
}

bool createVulkanDevice(VulkanContext &ctx) {
    float qprio = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = ctx.queueFamilyIndex;
    qci.queueCount = 1;
    qci.pQueuePriorities = &qprio;

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
    vkGetDeviceQueue(ctx.device, ctx.queueFamilyIndex, 0, &ctx.queue);
    std::cout << "[VK] step: device + queue created (swapchain ext enabled)\n";
    return true;
}

bool createSwapchain(VulkanContext &ctx, int width, int height, bool vsync) {
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physical, ctx.surface, &caps);

    uint32_t fmtCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physical, ctx.surface, &fmtCount, nullptr);
    if (fmtCount == 0) {
        std::cerr << "[VK] No surface formats\n";
        return false;
    }
    std::vector<VkSurfaceFormatKHR> fmts(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physical, ctx.surface, &fmtCount, fmts.data());
    ctx.swapFormat = fmts[0].format;

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
    if (!vsync)
    {	    
    	for (auto pm : pmodes) {
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = pm; break; }
        }
    }

    if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
        std::cout <<"[VK] VSYNC is enabled\n";
    else
    std::cout <<"[VK] VSYNC is disabled\n";    

    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = ctx.surface;
    sci.minImageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && sci.minImageCount > caps.maxImageCount)
        sci.minImageCount = caps.maxImageCount;
    sci.imageFormat = ctx.swapFormat;
    sci.imageColorSpace = fmts[0].colorSpace;
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
    std::cout << "[VK] step: swapchain created with " << imgCount << " images\n";
    return true;
}

bool createImageViews(VulkanContext &ctx) {
    ctx.imageViews.resize(ctx.images.size());
    for (size_t i = 0; i < ctx.images.size(); ++i) {
        VkImageViewCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image = ctx.images[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = ctx.swapFormat;
        ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.baseMipLevel = 0;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.baseArrayLayer = 0;
        ci.subresourceRange.layerCount = 1;

        VkResult r = vkCreateImageView(ctx.device, &ci, nullptr, &ctx.imageViews[i]);
        if (r != VK_SUCCESS) {
            std::cerr << "[VK] vkCreateImageView failed: " << r << "\n";
            return false;
        }
    }
    std::cout << "[VK] step: image views created\n";
    return true;
}

bool createRenderPass(VulkanContext &ctx) {
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

    VkResult r = vkCreateRenderPass(ctx.device, &rpci, nullptr, &ctx.renderPass);
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateRenderPass failed: " << r << "\n";
        return false;
    }
    std::cout << "[VK] step: render pass created\n";
    return true;
}

bool createFramebuffers(VulkanContext &ctx) {
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

        VkResult r = vkCreateFramebuffer(ctx.device, &fci, nullptr, &ctx.framebuffers[i]);
        if (r != VK_SUCCESS) {
            std::cerr << "[VK] vkCreateFramebuffer failed: " << r << "\n";
            return false;
        }
    }
    std::cout << "[VK] step: framebuffers created\n";
    return true;
}

bool createCommandPool(VulkanContext &ctx) {
    VkCommandPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.queueFamilyIndex = ctx.queueFamilyIndex;
    ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult r = vkCreateCommandPool(ctx.device, &ci, nullptr, &ctx.cmdPool);
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkCreateCommandPool failed: " << r << "\n";
        return false;
    }
    std::cout << "[VK] step: command pool created\n";
    return true;
}

bool allocateCommandBuffers(VulkanContext &ctx) {
    ctx.cmdBuffers.resize(ctx.images.size());
    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = ctx.cmdPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = (uint32_t)ctx.cmdBuffers.size();
    VkResult r = vkAllocateCommandBuffers(ctx.device, &ai, ctx.cmdBuffers.data());
    if (r != VK_SUCCESS) {
        std::cerr << "[VK] vkAllocateCommandBuffers failed: " << r << "\n";
        return false;
    }
    std::cout << "[VK] step: command buffers allocated\n";
    return true;
}

bool createSyncObjects(VulkanContext &ctx) {
    const size_t count = ctx.images.size();
    ctx.imageAvailable.resize(count);
    ctx.renderFinished.resize(count);
    ctx.inFlightFences.resize(count);

    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < count; ++i) {
        if (vkCreateSemaphore(ctx.device, &sci, nullptr, &ctx.imageAvailable[i]) != VK_SUCCESS ||
            vkCreateSemaphore(ctx.device, &sci, nullptr, &ctx.renderFinished[i]) != VK_SUCCESS ||
            vkCreateFence(ctx.device, &fci, nullptr, &ctx.inFlightFences[i]) != VK_SUCCESS)
        {
            std::cerr << "[VK] create sync objects failed\n";
            return false;
        }
    }
    std::cout << "[VK] step: sync objects created\n";
    return true;
}

// Record one command buffer per swapchain image with all tile clear commands
bool recordTileCommandBuffers(VulkanContext &ctx, const Args &args) {
    const int tileW = args.width / args.tilesX;
    const int tileH = args.height / args.tilesY;

    VkClearAttachment attachment{};
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = 0;
    attachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    for (size_t i = 0; i < ctx.cmdBuffers.size(); ++i) {
        VkCommandBuffer cmd = ctx.cmdBuffers[i];

        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(cmd, &bi) != VK_SUCCESS) {
            std::cerr << "[VK] vkBeginCommandBuffer failed\n";
            return false;
        }

        VkClearValue clearColor{};
        clearColor.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderPassBeginInfo rpBegin{};
        rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.renderPass = ctx.renderPass;
        rpBegin.framebuffer = ctx.framebuffers[i];
        rpBegin.renderArea.offset = { 0, 0 };
        rpBegin.renderArea.extent = ctx.extent;
        rpBegin.clearValueCount = 1;
        rpBegin.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        for (int ty = 0; ty < args.tilesY; ++ty) {
            for (int tx = 0; tx < args.tilesX; ++tx) {
                int x = tx * tileW;
                int y = ty * tileH;

                float r = (float)tx / (float)args.tilesX;
                float g = (float)ty / (float)args.tilesY;
                float b = 0.5f;
                attachment.clearValue.color.float32[0] = r;
                attachment.clearValue.color.float32[1] = g;
                attachment.clearValue.color.float32[2] = b;
                attachment.clearValue.color.float32[3] = 1.0f;

                VkClearRect rect{};
                rect.rect.offset = { x, y };
                uint32_t w = (tx == args.tilesX - 1)
                            ? (ctx.extent.width  - x)
                                : (uint32_t)tileW;

                uint32_t h = (ty == args.tilesY - 1)
                            ? (ctx.extent.height - y)
                                : (uint32_t)tileH;

                rect.rect.extent = { w, h };

                rect.baseArrayLayer = 0;
                rect.layerCount = 1;

                vkCmdClearAttachments(cmd, 1, &attachment, 1, &rect);
            }
        }

        vkCmdEndRenderPass(cmd);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            std::cerr << "[VK] vkEndCommandBuffer failed\n";
            return false;
        }
    }
    std::cout << "[VK] step: tile command buffers recorded\n";
    return true;
}

void destroyVulkan(VulkanContext &ctx) {
    if (ctx.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(ctx.device);

        for (auto f : ctx.inFlightFences) if (f) vkDestroyFence(ctx.device, f, nullptr);
        for (auto s : ctx.renderFinished) if (s) vkDestroySemaphore(ctx.device, s, nullptr);
        for (auto s : ctx.imageAvailable) if (s) vkDestroySemaphore(ctx.device, s, nullptr);

        if (!ctx.cmdBuffers.empty() && ctx.cmdPool != VK_NULL_HANDLE)
            vkFreeCommandBuffers(ctx.device, ctx.cmdPool,
                                 (uint32_t)ctx.cmdBuffers.size(),
                                 ctx.cmdBuffers.data());
        if (ctx.cmdPool) vkDestroyCommandPool(ctx.device, ctx.cmdPool, nullptr);

        for (auto fb : ctx.framebuffers) if (fb) vkDestroyFramebuffer(ctx.device, fb, nullptr);
        if (ctx.renderPass) vkDestroyRenderPass(ctx.device, ctx.renderPass, nullptr);

        for (auto iv : ctx.imageViews) if (iv) vkDestroyImageView(ctx.device, iv, nullptr);
        if (ctx.swapchain) vkDestroySwapchainKHR(ctx.device, ctx.swapchain, nullptr);

        vkDestroyDevice(ctx.device, nullptr);
    }

    if (ctx.surface && ctx.instance) vkDestroySurfaceKHR(ctx.instance, ctx.surface, nullptr);
    if (ctx.instance) vkDestroyInstance(ctx.instance, nullptr);
}

void runVulkanTiles(const Args &args)
{
    WaylandApp wl{};
    if (!initWayland(wl)) return;

    VulkanContext ctx{};
    if (!initVulkanInstance(ctx)) return;
    if (!initVulkanSurface(ctx, wl)) { destroyVulkan(ctx); return; }
    if (!pickVulkanDeviceAndQueue(ctx)) { destroyVulkan(ctx); return; }
    if (!createVulkanDevice(ctx)) { destroyVulkan(ctx); return; }
    if (!createSwapchain(ctx, args.width, args.height, args.vsync)) { destroyVulkan(ctx); return; }
    if (!createImageViews(ctx)) { destroyVulkan(ctx); return; }
    if (!createRenderPass(ctx)) { destroyVulkan(ctx); return; }
    if (!createFramebuffers(ctx)) { destroyVulkan(ctx); return; }
    if (!createCommandPool(ctx)) { destroyVulkan(ctx); return; }
    if (!allocateCommandBuffers(ctx)) { destroyVulkan(ctx); return; }
    if (!createSyncObjects(ctx)) { destroyVulkan(ctx); return; }
    if (!recordTileCommandBuffers(ctx, args)) { destroyVulkan(ctx); return; }

    std::cout << "[VK] Tiles: "
              << args.tilesX << " x " << args.tilesY
              << " (" << args.tilesX * args.tilesY << " clears per frame)\n";

    /* ---- PRIME CPU (fix NaN) ---- */
    getCPUUsage();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    getCPUUsage();

    auto start = std::chrono::high_resolution_clock::now();

    /* ---- stats ---- */
    double latencySumMs = 0.0;
    double maxLatencyMs = 0.0;
    int measuredFrames = 0;
    int framesThisSecond = 0;
    double cpuSum = 0.0;
    int cpuN = 0;

    const int WARMUP_FRAMES = 1;

    auto lastReportTime = start;
    int frame = 0;
    bool overlayReady = false;

    while (true) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        uint32_t imageIndex = 0;
        if (vkAcquireNextImageKHR(ctx.device, ctx.swapchain, UINT64_MAX,
                                  ctx.imageAvailable[frame % ctx.images.size()],
                                  VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS)
            break;

        VkFence fence = ctx.inFlightFences[imageIndex];
        vkWaitForFences(ctx.device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkResetFences(ctx.device, 1, &fence);

        VkSemaphore waitSem = ctx.imageAvailable[frame % ctx.images.size()];
        VkSemaphore signalSem = ctx.renderFinished[frame % ctx.images.size()];
        VkPipelineStageFlags waitStage =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &waitSem;
        submit.pWaitDstStageMask = &waitStage;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &ctx.cmdBuffers[imageIndex];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &signalSem;

        vkQueueSubmit(ctx.queue, 1, &submit, fence);

        VkPresentInfoKHR present{};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &signalSem;
        present.swapchainCount = 1;
        present.pSwapchains = &ctx.swapchain;
        present.pImageIndices = &imageIndex;

        vkQueuePresentKHR(ctx.queue, &present);

        vkQueueWaitIdle(ctx.queue);   // GL equivalent of glFinish()
        if (overlay && !overlayReady) 
	{
        	overlayInit(wl.display, wl.compositor, wl.shell,EGL_NO_DISPLAY, EGL_NO_CONTEXT,
                    args.width, args.height);
        overlayShowInfo("TILES BENCH","VULKAN", args.tilesX);
        overlayReady = true;
	}


        auto frameEnd = std::chrono::high_resolution_clock::now();

        if (frame >= WARMUP_FRAMES) {
            double latency =
                std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
            latencySumMs += latency;
            if (latency > maxLatencyMs) maxLatencyMs = latency;
            measuredFrames++;
        }

        framesThisSecond++;
        frame++;

        auto now = std::chrono::high_resolution_clock::now();
        double elapsed =
            std::chrono::duration<double>(now - lastReportTime).count();

        if (elapsed >= 1.0) {
            double cpu = getCPUUsage();
            double fps = framesThisSecond / elapsed;
            double avgLat =
                measuredFrames ? (latencySumMs / measuredFrames) : 0.0;

            std::cout << "[VK] FPS: " << fps
                      << " | CPU: " << cpu << "%"
                      << " | Avg Latency: " << avgLat << " ms"
                      << " | Max Latency: " << maxLatencyMs << " ms\n";

            if (overlay) overlayUpdatePerf(fps, cpu);

            appendCSV(args.csvFile, "Vulkan",
                     args.width, args.height, args.tilesX, args.tilesY, fps, cpu);            cpuSum += cpu;
            cpuN++;

            framesThisSecond = 0;
            lastReportTime = now;
        }

        capFrameTime(args.fpsCap, frameStart);

        double totalRuntime =
            std::chrono::duration<double>(now - start).count();      
        if (totalRuntime > runtime)
        {
            std::cout << "Exiting App as " << runtime << " seconds is reached \n";
            break;
        }
    }

    /* ---- FINAL STATS ---- */
    auto end = std::chrono::high_resolution_clock::now();
    double totalSec =
        std::chrono::duration<double>(end - start).count();

    int effectiveFrames = frame - WARMUP_FRAMES;
    if (effectiveFrames < 1) effectiveFrames = 1;

    double avgFPS = effectiveFrames / totalSec;
    double avgCPU = cpuN ? (cpuSum / cpuN) : 0.0;
    double avgLatency =
        measuredFrames ? (latencySumMs / measuredFrames) : 0.0;

    std::cout << "[VK] Average FPS " << avgFPS << "\n";
    std::cout << "[VK] Average CPU " << avgCPU << "\n";
    std::cout << "[VK] Average Frame Latency: " << avgLatency << " ms\n";
    std::cout << "[VK] Worst Frame Latency: " << maxLatencyMs << " ms\n";
    std::cout << "[VK] Total Frames Rendered : " << frame << "\n";

    appendCSV(args.csvFile, "Vulkan",
              args.width, args.height, args.tilesX, args.tilesY, avgFPS, avgCPU);

    if (overlay) 
        overlayShutdown();
    destroyVulkan(ctx);

    if (wl.surface) wl_proxy_destroy((wl_proxy*)wl.surface);
    if (wl.compositor) wl_proxy_destroy((wl_proxy*)wl.compositor);
    if (wl.registry) wl_proxy_destroy((wl_proxy*)wl.registry);
    if (wl.display) wl_display_disconnect(wl.display);
}


int main(int argc, char** argv) {
    Args args = parseArgs(argc, argv);

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--vulkan"))
            args.api = Args::API::Vulkan;
        else if (!strcmp(argv[i], "--no-overlay"))
            overlay = false;
    }
	
    std::cout << "tiles_bench: API="
              << (args.api == Args::API::GL ? "OpenGL" : "Vulkan")
              << " runtime=" << runtime
              << " size=" << args.width << "x" << args.height
              << " tiles=" << args.tilesX << "x" << args.tilesY << "\n";

    if (args.api == Args::API::GL)
        runGLTiles(args);
    else
        runVulkanTiles(args);

    return 0;
}
