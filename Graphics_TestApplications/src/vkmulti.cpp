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

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include "overlay.h"
#include <EGL/egl.h>
#include <simpleshell-client-protocol.h>

// --------------------------------------------------
// Defaults (override via CLI)
// --------------------------------------------------
int WIDTH  = 1920;
int HEIGHT = 1080;
int THREADS = 4;
int RECTS = 5000;
double RUN_TIME = 30.0;
static const int MAX_FRAMES = 2;
bool overlay = true;
std::string csvfile = "multithread_test.csv";

// --------------------------------------------------
// CLI
// --------------------------------------------------
void parseArgs(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--threads") && i + 1 < argc)
            THREADS = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--rects") && i + 1 < argc)
            RECTS = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--time") && i + 1 < argc)
            RUN_TIME = std::max(1.0, atof(argv[++i]));
        else if (!strcmp(argv[i], "--width") && i + 1 < argc)
            WIDTH = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--height") && i + 1 < argc)
            HEIGHT = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--no-overlay"))
            overlay = false;
        else if (!strcmp(argv[i], "--csv") && i + 1 < argc)
            csvfile = argv[++i];
    }
}

void appendCSV(double fps, double cpuEnd)
{
    bool exists = access(csvfile.c_str(), F_OK) == 0;
    std::string api = "Vulkan";
    std::ofstream out(csvfile, std::ios::app);
    if (!exists) out << "API,Resolution,Rectangles,FPS,CPU_Usage\n";
    out << api << "," << WIDTH << "x" << HEIGHT << "," << RECTS << "," << fps << "," << cpuEnd << "\n";
}

// --------------------------------------------------
// CPU usage
// --------------------------------------------------
static unsigned long long prevTotal = 0, prevIdle = 0;

double getCPUUsage() {
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
    wl_simple_shell* shell{};
};

static void reg_add(void* data, wl_registry* r,
                    uint32_t id, const char* iface, uint32_t) {
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

bool initWayland(WL& wl) {
    wl.d = wl_display_connect(nullptr);
    if (!wl.d) return false;

    wl.r = wl_display_get_registry(wl.d);
    static wl_registry_listener l{reg_add, reg_rem};
    wl_registry_add_listener(wl.r, &l, &wl);
    wl_display_roundtrip(wl.d);

    wl.s = wl_compositor_create_surface(wl.c);
    return wl.s != nullptr;
}

// --------------------------------------------------
// Rect (push constant)
// padded for safety on ARM
// --------------------------------------------------
struct Rect {
    float x, y;
    float r, g, b;
    float pad; // padding
};

// --------------------------------------------------
// Vulkan context
// --------------------------------------------------
struct VK {
    VkInstance inst{};
    VkPhysicalDevice phys{};
    VkDevice dev{};
    VkQueue q{};
    uint32_t qIdx{};

    VkSurfaceKHR surf{};
    VkSwapchainKHR sc{};
    VkFormat fmt{};
    VkExtent2D ext{};

    std::vector<VkImage> imgs;
    std::vector<VkImageView> views;
    VkRenderPass rp{};
    std::vector<VkFramebuffer> fbs;

    VkPipelineLayout layout{};
    VkPipeline pipe{};

    VkCommandPool primaryPool{};
	std::vector<VkCommandPool> threadPools;
    std::vector<VkCommandBuffer> primary;
    std::vector<std::vector<VkCommandBuffer>> secondary;

    std::vector<VkSemaphore> imageAvail;
    std::vector<VkSemaphore> renderDone;
    std::vector<VkFence> fences;
};

// --------------------------------------------------
// File loader
// --------------------------------------------------
static std::vector<char> loadFile(const char* path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return {};
    }
    return std::vector<char>(
        std::istreambuf_iterator<char>(f),
        std::istreambuf_iterator<char>());
}

int main(int argc, char** argv)
{
    parseArgs(argc, argv);
    std::cout << "[CONFIG] threads=" << THREADS
              << " rects=" << RECTS
              << " time=" << RUN_TIME
              << " size=" << WIDTH << "x" << HEIGHT
              << "\n";
    // ---------------- Wayland ----------------
    WL wl{};
    if (!initWayland(wl)) {
        std::cerr << "Wayland init failed\n";
        return -1;
    }

    // ---------------- Vulkan init ----------------
    VK vk{};

    const char* instExts[] = {
        "VK_KHR_surface",
        "VK_KHR_wayland_surface"
    };

    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.enabledExtensionCount = 2;
    ici.ppEnabledExtensionNames = instExts;
    VkResult result = vkCreateInstance(&ici, nullptr, &vk.inst);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance: " << result << std::endl;
        return -1;
    }

    VkWaylandSurfaceCreateInfoKHR sci{
        VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};
    sci.display = wl.d;
    sci.surface = wl.s;
    result = vkCreateWaylandSurfaceKHR(vk.inst, &sci, nullptr, &vk.surf);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Wayland surface: " << result << std::endl;
        return -1;
    }

    uint32_t pdCount = 0;
    vkEnumeratePhysicalDevices(vk.inst, &pdCount, nullptr);
    if (pdCount == 0) {
        std::cerr << "No Vulkan physical devices found" << std::endl;
        return -1;
    }
    std::vector<VkPhysicalDevice> pds(pdCount);
    vkEnumeratePhysicalDevices(vk.inst, &pdCount, pds.data());

    bool deviceFound = false;
    for (auto d : pds) {
        uint32_t qc = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qc, nullptr);
        std::vector<VkQueueFamilyProperties> qps(qc);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qc, qps.data());
        for (uint32_t i = 0; i < qc; ++i) {
            VkBool32 ok = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(d, i, vk.surf, &ok);
            if (ok && (qps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                vk.phys = d;
                vk.qIdx = i;
                deviceFound = true;
                goto found;
            }
        }
    }
found:
    if (!deviceFound) {
        std::cerr << "No suitable Vulkan device found" << std::endl;
        return -1;
    }

    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = vk.qIdx;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;

    const char* devExts[] = { "VK_KHR_swapchain" };
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = devExts;
    result = vkCreateDevice(vk.phys, &dci, nullptr, &vk.dev);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan device: " << result << std::endl;
        return -1;
    }
    vkGetDeviceQueue(vk.dev, vk.qIdx, 0, &vk.q);

    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.phys, vk.surf, &caps);
    vk.ext = caps.currentExtent.width != UINT32_MAX ?
             caps.currentExtent :
             VkExtent2D{WIDTH, HEIGHT};

    uint32_t fmtCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.phys, vk.surf,
                                         &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> fmts(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.phys, vk.surf,
                                         &fmtCount, fmts.data());
    vk.fmt = fmts[0].format;

    VkSwapchainCreateInfoKHR sc{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    sc.surface = vk.surf;
    sc.minImageCount = caps.minImageCount + 1;
    sc.imageFormat = vk.fmt;
    sc.imageColorSpace = fmts[0].colorSpace;
    sc.imageExtent = vk.ext;
    sc.imageArrayLayers = 1;
    sc.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sc.preTransform = caps.currentTransform;
    sc.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sc.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sc.clipped = VK_TRUE;
    vkCreateSwapchainKHR(vk.dev, &sc, nullptr, &vk.sc);

    uint32_t imgCount = 0;
    vkGetSwapchainImagesKHR(vk.dev, vk.sc, &imgCount, nullptr);
    vk.imgs.resize(imgCount);
    vkGetSwapchainImagesKHR(vk.dev, vk.sc,
                            &imgCount, vk.imgs.data());

    vk.views.resize(imgCount);
    for (uint32_t i = 0; i < imgCount; ++i) {
        VkImageViewCreateInfo iv{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        iv.image = vk.imgs[i];
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format = vk.fmt;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.levelCount = 1;
        iv.subresourceRange.layerCount = 1;
        vkCreateImageView(vk.dev, &iv, nullptr, &vk.views[i]);
    }

    VkAttachmentDescription ad{};
    ad.format = vk.fmt;
    ad.samples = VK_SAMPLE_COUNT_1_BIT;
    ad.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ad.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ad.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ar{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription sp{};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount = 1;
    sp.pColorAttachments = &ar;

    // *** REQUIRED on RTD1325 ***
    // Explicit external → subpass dependency
    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rp.attachmentCount = 1;
    rp.pAttachments = &ad;
    rp.subpassCount = 1;
    rp.pSubpasses = &sp;
    rp.dependencyCount = 1;
    rp.pDependencies = &dep;
    vkCreateRenderPass(vk.dev, &rp, nullptr, &vk.rp);

    vk.fbs.resize(imgCount);
    for (uint32_t i = 0; i < imgCount; ++i) {
        VkFramebufferCreateInfo fb{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fb.renderPass = vk.rp;
        fb.attachmentCount = 1;
        fb.pAttachments = &vk.views[i];
        fb.width = vk.ext.width;
        fb.height = vk.ext.height;
        fb.layers = 1;
        vkCreateFramebuffer(vk.dev, &fb, nullptr, &vk.fbs[i]);
    }

    VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	pci.queueFamilyIndex = vk.qIdx;
	pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	// Primary pool
	vkCreateCommandPool(vk.dev, &pci, nullptr, &vk.primaryPool);

	// Thread pools
	vk.threadPools.resize(THREADS);
	for (int i = 0; i < THREADS; i++) {
		vkCreateCommandPool(
			vk.dev,
			&pci,
			nullptr,
			&vk.threadPools[i]);
	}

    // ---------------- Pipeline ----------------
    auto vertCode = loadFile("rect.vert.spv");
    auto fragCode = loadFile("rect.frag.spv");
    
    if (vertCode.empty() || fragCode.empty()) {
        std::cerr << "Failed to load shader files. Ensure rect.vert.spv and rect.frag.spv exist." << std::endl;
        return -1;
    }

    VkShaderModuleCreateInfo sm{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    sm.codeSize = vertCode.size();
    sm.pCode = (uint32_t*)vertCode.data();
    VkShaderModule vert;
    result = vkCreateShaderModule(vk.dev, &sm, nullptr, &vert);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create vertex shader module: " << result << std::endl;
        return -1;
    }

    sm.codeSize = fragCode.size();
    sm.pCode = (uint32_t*)fragCode.data();
    VkShaderModule frag;
    result = vkCreateShaderModule(vk.dev, &sm, nullptr, &frag);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create fragment shader module: " << result << std::endl;
        return -1;
    }

    VkPushConstantRange pc{};
    pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pc.size = sizeof(Rect);

    VkPipelineLayoutCreateInfo lci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    lci.pushConstantRangeCount = 1;
    lci.pPushConstantRanges = &pc;
    vkCreatePipelineLayout(vk.dev, &lci, nullptr, &vk.layout);

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vert;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = frag;
    stages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport vp{0,0,(float)vk.ext.width,(float)vk.ext.height,0,1};
    VkRect2D scissor{{0,0},vk.ext};
    VkPipelineViewportStateCreateInfo vpci{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vpci.viewportCount = 1; vpci.pViewports = &vp;
    vpci.scissorCount = 1;  vpci.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState cba{};
    cba.colorWriteMask = 0xF;
    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1;
    cb.pAttachments = &cba;

    VkGraphicsPipelineCreateInfo gp{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    gp.stageCount = 2;
    gp.pStages = stages;
    gp.pVertexInputState = &vi;
    gp.pInputAssemblyState = &ia;
    gp.pViewportState = &vpci;
    gp.pRasterizationState = &rs;
    gp.pMultisampleState = &ms;
    gp.pColorBlendState = &cb;
    gp.layout = vk.layout;
    gp.renderPass = vk.rp;
    vkCreateGraphicsPipelines(vk.dev, VK_NULL_HANDLE, 1, &gp, nullptr, &vk.pipe);

    vkDestroyShaderModule(vk.dev, vert, nullptr);
    vkDestroyShaderModule(vk.dev, frag, nullptr);

    // ---------------- Sync + CBs ----------------
    vk.imageAvail.resize(MAX_FRAMES);
    vk.renderDone.resize(MAX_FRAMES);
    vk.fences.resize(MAX_FRAMES);

    VkSemaphoreCreateInfo sci2{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES; i++) {
        vkCreateSemaphore(vk.dev, &sci2, nullptr, &vk.imageAvail[i]);
        vkCreateSemaphore(vk.dev, &sci2, nullptr, &vk.renderDone[i]);
        vkCreateFence(vk.dev, &fci, nullptr, &vk.fences[i]);
    }

    vk.primary.resize(MAX_FRAMES);
    vk.secondary.resize(MAX_FRAMES);

    VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    ai.commandPool = vk.primaryPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = MAX_FRAMES;
    vkAllocateCommandBuffers(vk.dev, &ai, vk.primary.data());

    ai.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	for (int f = 0; f < MAX_FRAMES; f++) {
	
		vk.secondary[f].resize(THREADS);
	
		for (int th = 0; th < THREADS; th++) {
	
			ai.commandPool = vk.threadPools[th];
			ai.commandBufferCount = 1;
	
			vkAllocateCommandBuffers(
				vk.dev,
				&ai,
				&vk.secondary[f][th]);
		}
	}

    // ---------------- Rects ----------------
    std::vector<Rect> rects(RECTS);
    for (auto& r : rects) {
        r.x = float(rand() % WIDTH - WIDTH/2);
        r.y = float(rand() % HEIGHT - HEIGHT/2);
        r.r = rand() / float(RAND_MAX);
        r.g = rand() / float(RAND_MAX);
        r.b = rand() / float(RAND_MAX);
        r.pad = 0.0f;
    }

    auto start = std::chrono::steady_clock::now();
    auto lastStat = start;
    int frames = 0;
    getCPUUsage();

    // Variables for average calculation
    double totalFPS = 0.0;
    double totalCPU = 0.0;
    int sampleCount = 0;

    uint32_t frameIdx = 0;
	bool overlayReady = false;

    // ---------------- Render loop ----------------
    while (true) {
        auto now = std::chrono::steady_clock::now();
        double t = std::chrono::duration<double>(now - start).count();
        if (t > RUN_TIME) break;

        uint32_t cur = frameIdx++ % MAX_FRAMES;

        vkWaitForFences(vk.dev, 1, &vk.fences[cur], VK_TRUE, UINT64_MAX);
        vkResetFences(vk.dev, 1, &vk.fences[cur]);

        uint32_t imgIdx;
        vkAcquireNextImageKHR(vk.dev, vk.sc, UINT64_MAX,
                              vk.imageAvail[cur], VK_NULL_HANDLE, &imgIdx);

        // --- secondary (multithreaded) ---
        std::vector<std::thread> workers;
        for (int th = 0; th < THREADS; th++) {
            workers.emplace_back([&, th] {
                VkCommandBuffer cmd = vk.secondary[cur][th];

                VkCommandBufferInheritanceInfo inh{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
				inh.renderPass = vk.rp;
				inh.framebuffer = vk.fbs[imgIdx];
				inh.subpass = 0;

                VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                bi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                bi.pInheritanceInfo = &inh;

                vkResetCommandBuffer(cmd, 0);
				vkBeginCommandBuffer(cmd, &bi);
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipe);

                for (int i = th; i < RECTS; i += THREADS) {
                    Rect r = rects[i];
                    r.x += sin(t + i) * 20.0f;
                    vkCmdPushConstants(cmd, vk.layout,
                        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Rect), &r);
                    vkCmdDraw(cmd, 6, 1, 0, 0);
                }
                vkEndCommandBuffer(cmd);
            });
        }
        for (auto& th : workers) th.join();

        // --- primary ---
        VkCommandBuffer cmd = vk.primary[cur];
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        vkBeginCommandBuffer(cmd, &bi);

        VkClearValue cv{{{0.05f,0.05f,0.05f,1}}};
        VkRenderPassBeginInfo rpb{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rpb.renderPass = vk.rp;
        rpb.framebuffer = vk.fbs[imgIdx];
        rpb.renderArea.extent = vk.ext;
        rpb.clearValueCount = 1;
        rpb.pClearValues = &cv;

        vkCmdBeginRenderPass(cmd, &rpb,
            VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vkCmdExecuteCommands(cmd, THREADS, vk.secondary[cur].data());
        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);

        VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.waitSemaphoreCount = 1;
        si.pWaitSemaphores = &vk.imageAvail[cur];
        si.pWaitDstStageMask = &stage;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &cmd;
        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores = &vk.renderDone[cur];
        vkQueueSubmit(vk.q, 1, &si, vk.fences[cur]);

        VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = &vk.renderDone[cur];
        pi.swapchainCount = 1;
        pi.pSwapchains = &vk.sc;
        pi.pImageIndices = &imgIdx;
        vkQueuePresentKHR(vk.q, &pi);

        if (overlay && !overlayReady) {
            overlayInit(
                wl.d,
                wl.c,
                wl.shell,
                EGL_NO_DISPLAY,
                EGL_NO_CONTEXT,
                WIDTH,
                HEIGHT
            );
            overlayShowInfo("MULTI THREADING TEST","VULKAN",RECTS);
            overlayReady = true;
        }
	
	frames++;
        double sec = std::chrono::duration<double>(now - lastStat).count();
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

    vkDeviceWaitIdle(vk.dev);
    
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
    std::cout << "Exited cleanly\n";
    return 0;
}
