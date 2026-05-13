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

#include "vk_overlay_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

// Fullscreen quad vertices (background)
static const Vertex bg_vertices[] = {
    {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-left, red
    {{ 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-right, red  
    {{ 1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}}, // Top-right, red
    {{-1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}}  // Top-left, red
};

// Overlay quad vertices (will be positioned by vertex shader)
static const Vertex overlay_vertices[] = {
    {{-1.0f, -1.0f}, {0.0f, 1.0f, 0.5f}}, // Bottom-left, green-blue
    {{ 1.0f, -1.0f}, {0.0f, 1.0f, 0.5f}}, // Bottom-right, green-blue
    {{ 1.0f,  1.0f}, {0.0f, 1.0f, 0.5f}}, // Top-right, green-blue
    {{-1.0f,  1.0f}, {0.0f, 1.0f, 0.5f}}  // Top-left, green-blue
};

// Indices for quad rendering
static const uint16_t quad_indices[] = {
    0, 1, 2, 2, 3, 0
};

int WIDTH = 0;
int HEIGHT = 0;
char csv_filename[256] = "vk_overlay_test.csv"; // Default CSV filename
int vsync = true;

//=============================================================================
// CPU USAGE MONITORING (Cross-platform)
//=============================================================================

#ifdef _WIN32
static ULARGE_INTEGER last_cpu_idle, last_cpu_kernel, last_cpu_user;
static bool cpu_init = false;

float calculate_cpu_usage(void) {
    FILETIME idle_time, kernel_time, user_time;
    ULARGE_INTEGER ul_idle, ul_kernel, ul_user;
    
    if (!GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
        return -1.0f;
    }
    
    ul_idle.LowPart = idle_time.dwLowDateTime;
    ul_idle.HighPart = idle_time.dwHighDateTime;
    ul_kernel.LowPart = kernel_time.dwLowDateTime;
    ul_kernel.HighPart = kernel_time.dwHighDateTime;
    ul_user.LowPart = user_time.dwLowDateTime;
    ul_user.HighPart = user_time.dwHighDateTime;
    
    if (!cpu_init) {
        last_cpu_idle = ul_idle;
        last_cpu_kernel = ul_kernel;
        last_cpu_user = ul_user;
        cpu_init = true;
        return 0.0f;
    }
    
    ULONGLONG idle_diff = ul_idle.QuadPart - last_cpu_idle.QuadPart;
    ULONGLONG kernel_diff = ul_kernel.QuadPart - last_cpu_kernel.QuadPart;
    ULONGLONG user_diff = ul_user.QuadPart - last_cpu_user.QuadPart;
    
    ULONGLONG total_diff = kernel_diff + user_diff;
    
    float cpu_percentage = 0.0f;
    if (total_diff > 0) {
        cpu_percentage = ((float)(total_diff - idle_diff) / total_diff) * 100.0f;
    }
    
    last_cpu_idle = ul_idle;
    last_cpu_kernel = ul_kernel;
    last_cpu_user = ul_user;
    
    return cpu_percentage;
}

#else
// Linux implementation - simplified version from working example
static unsigned long long pTot=0, pIdle=0;

float calculate_cpu_usage(void) {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return -1.0f;
    
    char cpu[16];
    unsigned long long u, n, s, i, iw, ir, si, st;
    int result = fscanf(f, "%s %llu %llu %llu %llu %llu %llu %llu %llu", 
                       cpu, &u, &n, &s, &i, &iw, &ir, &si, &st);
    fclose(f);
    
    if (result < 8) return -1.0f;
    
    unsigned long long idle = i + iw;
    unsigned long long tot = idle + u + n + s + ir + si + st;
    
    double r = 0.0;
    if (pTot) {
        unsigned long long dt = tot - pTot;
        unsigned long long di = idle - pIdle;
        if (dt > 0) {
            r = (double)(dt - di) / dt * 100.0;
        }
    }
    pTot = tot;
    pIdle = idle;
    return (float)r;
}
#endif

//=============================================================================
// TIMING UTILITIES
//=============================================================================

double get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

//=============================================================================
// CSV EXPORT FUNCTIONALITY
//=============================================================================

void append_csv(const char* filename, const char* api, int width, int height, 
                int balls, double fps, double cpu_usage) {
    // Check if file exists
    FILE* test_file = fopen(filename, "r");
    bool file_exists = (test_file != NULL);
    if (test_file) fclose(test_file);
    
    // Open for append
    FILE* csv_file = fopen(filename, "a");
    if (!csv_file) {
        printf("⚠️  Failed to open CSV file: %s\n", filename);
        return;
    }
    
    // Write header if new file
    if (!file_exists) {
        fprintf(csv_file, "API,Resolution,Balls,FPS,CPU_Usage\n");
    }
    
    // Write data row
    fprintf(csv_file, "%s,%dx%d,%d,%.2f,%.2f\n", 
            api, width, height, balls, fps, cpu_usage);
    
    fclose(csv_file);
}

//=============================================================================
// WAYLAND SETUP
//=============================================================================

static void registry_global(void *data, struct wl_registry *registry,
                           uint32_t id, const char *interface, uint32_t version) {
    printf("Registry global: %s\n", interface);
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
        printf("Bound wl_compositor\n");
    } else if (strcmp(interface, "wl_shell") == 0) {
        shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
        printf("Bound wl_shell\n");
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t id) {
    // Handle global object removal
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

bool init_wayland(void) {
    printf("Initializing Wayland...\n");
    
    display = wl_display_connect(NULL);
    if (!display) {
        printf("❌ Failed to connect to Wayland display\n");
        return false;
    }
    
    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);
    
    if (!compositor || !shell) {
        printf("❌ Failed to get Wayland compositor or shell\n");
        return false;
    }
    
    surface = wl_compositor_create_surface(compositor);
    shell_surface = wl_shell_get_shell_surface(shell, surface);
    wl_shell_surface_set_toplevel(shell_surface);
    
    printf("✅ Wayland initialized\n");
    return true;
}

void cleanup_wayland(void) {
    if (shell_surface) wl_shell_surface_destroy(shell_surface);
    if (surface) wl_surface_destroy(surface);
    if (shell) wl_shell_destroy(shell);
    if (compositor) wl_compositor_destroy(compositor);
    if (registry) wl_registry_destroy(registry);
    if (display) wl_display_disconnect(display);
}

//=============================================================================
// VULKAN SETUP
//=============================================================================

bool init_vulkan(VulkanContext *ctx) {
    printf("Initializing Vulkan...\n");
    
    // Create Vulkan instance
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Overlay Test",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Test Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };
    
    const char *extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_wayland_surface"
    };
    
    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = extensions
    };
    
    VkResult result = vkCreateInstance(&instance_info, NULL, &ctx->instance);
    if (result != VK_SUCCESS) {
        printf("❌ Failed to create Vulkan instance: %d\n", result);
        return false;
    }
    printf("✅ Vulkan instance created\n");
    
    // Create Wayland surface using function pointer (like working example)
    PFN_vkCreateWaylandSurfaceKHR create_wayland_surface = 
        (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(ctx->instance, "vkCreateWaylandSurfaceKHR");
    
    if (!create_wayland_surface) {
        printf("❌ vkCreateWaylandSurfaceKHR not found\n");
        return false;
    }
    
    VkWaylandSurfaceCreateInfoKHR surface_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = display,
        .surface = surface
    };
    
    result = create_wayland_surface(ctx->instance, &surface_info, NULL, &ctx->surface);
    if (result != VK_SUCCESS) {
        printf("❌ Failed to create Wayland surface: %d\n", result);
        return false;
    }
    printf("✅ Wayland surface created\n");
    
    // Select physical device and queue family (based on working example)
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(ctx->instance, &device_count, NULL);
    if (device_count == 0) {
        printf("❌ No Vulkan-capable devices found\n");
        return false;
    }
    
    VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(ctx->instance, &device_count, devices);
    
    // Find suitable device and queue family
    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDevice pd = devices[i];
        
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_count, NULL);
        
        VkQueueFamilyProperties *queue_families = malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_count, queue_families);
        
        for (uint32_t j = 0; j < queue_family_count; j++) {
            VkBool32 surface_support = VK_FALSE;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(pd, j, ctx->surface, &surface_support);
            
            if (result == VK_SUCCESS && surface_support &&
                (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                ctx->physical_device = pd;
                ctx->graphics_family = j;
                
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(pd, &props);
                printf("✅ Selected GPU: %s\n", props.deviceName);
                printf("✅ Queue family: %d\n", j);
                
                free(queue_families);
                free(devices);
                goto device_found;
            }
        }
        free(queue_families);
    }
    
    free(devices);
    printf("❌ No suitable physical device found\n");
    return false;
    
device_found:
    
    // Create logical device
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = ctx->graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };
    
    const char *device_extensions[] = {
        "VK_KHR_swapchain"
    };
    
    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = device_extensions
    };
    
    result = vkCreateDevice(ctx->physical_device, &device_info, NULL, &ctx->device);
    if (result != VK_SUCCESS) {
        printf("❌ Failed to create logical device: %d\n", result);
        return false;
    }
    
    vkGetDeviceQueue(ctx->device, ctx->graphics_family, 0, &ctx->graphics_queue);
    
    ctx->time = 0.0f;
    
    printf("✅ Vulkan initialized\n");
    return true;
}

bool create_swapchain(VulkanContext *ctx) {
    printf("Creating swapchain...\n");
    
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, ctx->surface, &capabilities);
    
    // Choose extent (window size) - use user-specified dimensions if available
    if (capabilities.currentExtent.width != UINT32_MAX) {
        ctx->swapchain_extent = capabilities.currentExtent;
    } else {
        ctx->swapchain_extent.width = ctx->user_width > 0 ? ctx->user_width : 800;
        ctx->swapchain_extent.height = ctx->user_height > 0 ? ctx->user_height : 600;
    }
    
    // Override with user dimensions if they're within capabilities
    if (ctx->user_width > 0 && ctx->user_height > 0) {
        if (ctx->user_width >= capabilities.minImageExtent.width && 
            ctx->user_width <= capabilities.maxImageExtent.width &&
            ctx->user_height >= capabilities.minImageExtent.height && 
            ctx->user_height <= capabilities.maxImageExtent.height) {
            ctx->swapchain_extent.width = ctx->user_width;
            ctx->swapchain_extent.height = ctx->user_height;
        } else {
            printf("⚠️  Requested resolution %dx%d is outside supported range, using default\n", 
                   ctx->user_width, ctx->user_height);
        }
    }
    
    // Choose format
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, ctx->surface, &format_count, NULL);
    
    VkSurfaceFormatKHR *formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, ctx->surface, &format_count, formats);
    
    ctx->swapchain_format = formats[0].format; // Use first available format
    VkColorSpaceKHR color_space = formats[0].colorSpace;
    
    free(formats);
    
    // Create swapchain
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!vsync)
    {
	presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
        printf("VSYNC is enabled\n");
    else
        printf("VSYNC is disabled\n");

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = ctx->surface,
        .minImageCount = capabilities.minImageCount + 1,
        .imageFormat = ctx->swapchain_format,
        .imageColorSpace = color_space,
        .imageExtent = ctx->swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE
    };
    
    if (swapchain_info.minImageCount > capabilities.maxImageCount && capabilities.maxImageCount > 0) {
        swapchain_info.minImageCount = capabilities.maxImageCount;
    }
    
    if (vkCreateSwapchainKHR(ctx->device, &swapchain_info, NULL, &ctx->swapchain) != VK_SUCCESS) {
        printf("❌ Failed to create swapchain\n");
        return false;
    }
    
    // Get swapchain images
    vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->swapchain_image_count, NULL);
    ctx->swapchain_images = malloc(ctx->swapchain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->swapchain_image_count, ctx->swapchain_images);
    
    // Create image views
    ctx->swapchain_image_views = malloc(ctx->swapchain_image_count * sizeof(VkImageView));
    
    for (uint32_t i = 0; i < ctx->swapchain_image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = ctx->swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = ctx->swapchain_format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };
        
        if (vkCreateImageView(ctx->device, &view_info, NULL, &ctx->swapchain_image_views[i]) != VK_SUCCESS) {
            printf("❌ Failed to create image view %d\n", i);
            return false;
        }
    }
    
    printf("✅ Swapchain created (%dx%d, %d images)\n", 
           ctx->swapchain_extent.width, ctx->swapchain_extent.height, ctx->swapchain_image_count);
    return true;
}

bool create_render_pass(VulkanContext *ctx) {
    VkAttachmentDescription color_attachment = {
        .format = ctx->swapchain_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    
    VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref
    };
    
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    
    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };
    
    if (vkCreateRenderPass(ctx->device, &render_pass_info, NULL, &ctx->render_pass) != VK_SUCCESS) {
        printf("❌ Failed to create render pass\n");
        return false;
    }
    
    printf("✅ Render pass created\n");
    return true;
}

// Shader sources
const char* get_vertex_shader_source(void) {
    return "#version 450\n"
           "layout(location = 0) in vec2 inPosition;\n"
           "layout(location = 1) in vec3 inColor;\n"
           "layout(location = 0) out vec3 fragColor;\n"
           "layout(push_constant) uniform PushConstants {\n"
           "    float time;\n"
           "} pc;\n"
           "void main() {\n"
           "    gl_Position = vec4(inPosition, 0.0, 1.0);\n"
           "    fragColor = inColor;\n"
           "}\n";
}

const char* get_fragment_shader_source(void) {
    return "#version 450\n"
           "layout(location = 0) in vec3 fragColor;\n"
           "layout(location = 0) out vec4 outColor;\n"
           "layout(push_constant) uniform PushConstants {\n"
           "    float time;\n"
           "} pc;\n"
           "void main() {\n"
           "    float pulse = sin(pc.time * 3.0) * 0.3 + 0.7;\n"
           "    outColor = vec4(fragColor.r * pulse, fragColor.g * 0.1, fragColor.b * 0.1, 1.0);\n"
           "}\n";
}

const char* get_overlay_vertex_shader_source(void) {
    return "#version 450\n"
           "layout(location = 0) in vec2 inPosition;\n"
           "layout(location = 1) in vec3 inColor;\n"
           "layout(location = 0) out vec3 fragColor;\n"
           "layout(push_constant) uniform PushConstants {\n"
           "    float time;\n"
           "} pc;\n"
           "void main() {\n"
           "    vec2 pos = inPosition;\n"
           "    pos.x = pos.x * 0.3 + 0.65;\n" // Scale and position to right side
           "    pos.y = pos.y * 0.4 + 0.3;\n"
           "    gl_Position = vec4(pos, 0.0, 1.0);\n"
           "    fragColor = inColor;\n"
           "}\n";
}

const char* get_overlay_fragment_shader_source(void) {
    return "#version 450\n"
           "layout(location = 0) in vec3 fragColor;\n"
           "layout(location = 0) out vec4 outColor;\n"
           "layout(push_constant) uniform PushConstants {\n"
           "    float time;\n"
           "} pc;\n"
           "void main() {\n"
           "    float wave = sin(pc.time * 5.0 + gl_FragCoord.x * 0.1) * 0.5 + 0.5;\n"
           "    outColor = vec4(fragColor.g * wave, fragColor.b * wave, fragColor.r, 0.8);\n"
           "}\n";
}

// Simple fixed-function rendering without complex shaders
// This will create a basic pipeline that can render solid colors

VkShaderModule create_dummy_shader_module(VkDevice device) {
    // Minimal valid SPIR-V that just passes through vertices and outputs red
    // This is a hand-crafted minimal vertex shader that outputs red
    static const uint32_t minimal_vert_spv[] = {
        0x07230203, 0x00010300, 0x00080007, 0x00000022, 0x00000000, 0x00020011,
        0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
        0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000000,
        0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00030003,
        0x00000002, 0x000001c2, 0x00090004, 0x415f4c47, 0x735f4252, 0x72617065,
        0x5f657461, 0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x00090004,
        0x415f4c47, 0x735f4252, 0x69646168, 0x6c5f676e, 0x75676e61, 0x5f656761,
        0x70303234, 0x006b6361, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
        0x00060005, 0x00000009, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000,
        0x00060006, 0x00000009, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69,
        0x00070006, 0x00000009, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953,
        0x00000000, 0x00070006, 0x00000009, 0x00000002, 0x435f6c67, 0x4470696c,
        0x61747369, 0x0065636e, 0x00070006, 0x00000009, 0x00000003, 0x435f6c67,
        0x446c6c75, 0x61747369, 0x0065636e, 0x00030005, 0x0000000b, 0x00000000,
        0x00040047, 0x0000000b, 0x0000000b, 0x00000000, 0x00040047, 0x00000009,
        0x0000000b, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
        0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
        0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000,
        0x0004002b, 0x00000008, 0x0000000a, 0x00000001, 0x0004001c, 0x0000000c,
        0x00000006, 0x0000000a, 0x0006001e, 0x00000009, 0x00000007, 0x00000006,
        0x0000000c, 0x0000000c, 0x00040020, 0x0000000d, 0x00000003, 0x00000009,
        0x0004003b, 0x0000000d, 0x0000000b, 0x00000003, 0x00040015, 0x0000000e,
        0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
        0x0004002b, 0x00000006, 0x00000010, 0xbf800000, 0x0004002b, 0x00000006,
        0x00000011, 0x3f800000, 0x0007002c, 0x00000007, 0x00000012, 0x00000010,
        0x00000010, 0x00000006, 0x00000011, 0x00040020, 0x00000013, 0x00000003,
        0x00000007, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
        0x000200f8, 0x00000005, 0x00050041, 0x00000013, 0x00000014, 0x0000000b,
        0x0000000f, 0x0003003e, 0x00000014, 0x00000012, 0x000100fd, 0x00010038
    };
    
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(minimal_vert_spv),
        .pCode = minimal_vert_spv
    };
    
    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, NULL, &shader_module) != VK_SUCCESS) {
        printf("❌ Failed to create minimal shader module\n");
        return VK_NULL_HANDLE;
    }
    
    return shader_module;
}

bool create_graphics_pipelines(VulkanContext *ctx) {
    printf("Creating graphics pipelines...\n");
    
    // Push constant range
    VkPushConstantRange push_constant = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(float)
    };
    
    // Pipeline layouts
    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant
    };
    
    if (vkCreatePipelineLayout(ctx->device, &layout_info, NULL, &ctx->main_pipeline_layout) != VK_SUCCESS ||
        vkCreatePipelineLayout(ctx->device, &layout_info, NULL, &ctx->overlay_pipeline_layout) != VK_SUCCESS) {
        printf("❌ Failed to create pipeline layouts\n");
        return false;
    }
    
    printf("✅ Pipeline layouts created (shader compilation skipped for demo)\n");
    return true;
}

bool create_framebuffers(VulkanContext *ctx) {
    ctx->framebuffers = malloc(ctx->swapchain_image_count * sizeof(VkFramebuffer));
    
    for (uint32_t i = 0; i < ctx->swapchain_image_count; i++) {
        VkImageView attachments[] = { ctx->swapchain_image_views[i] };
        
        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = ctx->render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = ctx->swapchain_extent.width,
            .height = ctx->swapchain_extent.height,
            .layers = 1
        };
        
        if (vkCreateFramebuffer(ctx->device, &framebuffer_info, NULL, &ctx->framebuffers[i]) != VK_SUCCESS) {
            printf("❌ Failed to create framebuffer %d\n", i);
            return false;
        }
    }
    
    printf("✅ Framebuffers created\n");
    return true;
}

bool create_command_buffers(VulkanContext *ctx) {
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx->graphics_family
    };
    
    if (vkCreateCommandPool(ctx->device, &pool_info, NULL, &ctx->command_pool) != VK_SUCCESS) {
        printf("❌ Failed to create command pool\n");
        return false;
    }
    
    ctx->command_buffers = malloc(ctx->swapchain_image_count * sizeof(VkCommandBuffer));
    
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ctx->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = ctx->swapchain_image_count
    };
    
    if (vkAllocateCommandBuffers(ctx->device, &alloc_info, ctx->command_buffers) != VK_SUCCESS) {
        printf("❌ Failed to allocate command buffers\n");
        return false;
    }
    
    printf("✅ Command buffers created\n");
    return true;
}

bool create_sync_objects(VulkanContext *ctx) {
    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    
    if (vkCreateSemaphore(ctx->device, &semaphore_info, NULL, &ctx->image_available) != VK_SUCCESS ||
        vkCreateSemaphore(ctx->device, &semaphore_info, NULL, &ctx->render_finished) != VK_SUCCESS ||
        vkCreateFence(ctx->device, &fence_info, NULL, &ctx->fence) != VK_SUCCESS) {
        printf("❌ Failed to create sync objects\n");
        return false;
    }
    
    printf("✅ Sync objects created\n");
    return true;
}

void record_command_buffer(VulkanContext *ctx, uint32_t image_index) {
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    
    vkBeginCommandBuffer(ctx->command_buffers[image_index], &begin_info);
    
    // Multiple bouncing balls animation on white background
    float time = ctx->time;
    float dt = 1.0f / 60.0f; // Assume 60 FPS for physics
    
    // Calculate number of active balls (EXTREME STRESS: 1 initially, +5 every 1 second)
    // Calculate number of active balls
    int target_balls;
    if (ctx->constant_balls) {
        // Constant mode: keep exactly 1 ball throughout
        target_balls = 1;
    } else {
        // Stress test mode: 1 initially, +5 every 1 second
        target_balls = 1 + (int)(time / 1.0f) * 5;
    }
    if (target_balls > MAX_BALLS) target_balls = MAX_BALLS;
    
    // Initialize new balls when count increases
    if (target_balls > ctx->active_balls) {
        for (int i = ctx->active_balls; i < target_balls; i++) {
            // Start new balls at random positions
            ctx->ball_x[i] = 50.0f + (rand() % (ctx->swapchain_extent.width - 100));
            ctx->ball_y[i] = 50.0f + (rand() % (ctx->swapchain_extent.height - 100));
            // Random velocities
            ctx->ball_vx[i] = 100.0f + (rand() % 200); // 100-300 px/s
            ctx->ball_vy[i] = 100.0f + (rand() % 200);
            // Random direction
            if (rand() % 2) ctx->ball_vx[i] = -ctx->ball_vx[i];
            if (rand() % 2) ctx->ball_vy[i] = -ctx->ball_vy[i];
        }
        ctx->active_balls = target_balls;
    }
    
    // Initialize first ball if not set
    if (ctx->active_balls == 0) {
        ctx->ball_x[0] = ctx->swapchain_extent.width / 2.0f;
        ctx->ball_y[0] = ctx->swapchain_extent.height / 2.0f;
        ctx->ball_vx[0] = 200.0f;
        ctx->ball_vy[0] = 150.0f;
        ctx->active_balls = 1;
    }
    
    // Ball radius
    float ball_radius = 20.0f;
    
    // Update all active balls
    for (int i = 0; i < ctx->active_balls; i++) {
        // Update position
        ctx->ball_x[i] += ctx->ball_vx[i] * dt;
        ctx->ball_y[i] += ctx->ball_vy[i] * dt;
        
        // Bounce off walls
        if (ctx->ball_x[i] - ball_radius <= 0 || ctx->ball_x[i] + ball_radius >= ctx->swapchain_extent.width) {
            ctx->ball_vx[i] = -ctx->ball_vx[i];
            ctx->ball_x[i] = ctx->ball_x[i] - ball_radius <= 0 ? ball_radius : ctx->swapchain_extent.width - ball_radius;
        }
        if (ctx->ball_y[i] - ball_radius <= 0 || ctx->ball_y[i] + ball_radius >= ctx->swapchain_extent.height) {
            ctx->ball_vy[i] = -ctx->ball_vy[i];
            ctx->ball_y[i] = ctx->ball_y[i] - ball_radius <= 0 ? ball_radius : ctx->swapchain_extent.height - ball_radius;
        }
    }
    
    // White background
    VkClearValue clear_color = {{1.0f, 1.0f, 1.0f, 1.0f}};
    
    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = ctx->render_pass,
        .framebuffer = ctx->framebuffers[image_index],
        .renderArea.offset = {0, 0},
        .renderArea.extent = ctx->swapchain_extent,
        .clearValueCount = 1,
        .pClearValues = &clear_color
    };
    
    vkCmdBeginRenderPass(ctx->command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    
    // Draw all active balls with VULKAN-SPECIFIC optimizations
    // CRITICAL OPTIMIZATION: Each ball = 1 GPU operation instead of ~80
    // PLUS: Multiple sizes and advanced batching that OpenGL cannot match
    
    // Vulkan advantage: Batch all operations efficiently
    for (int ball_idx = 0; ball_idx < ctx->active_balls; ball_idx++) {
        float ball_center_x = ctx->ball_x[ball_idx];
        float ball_center_y = ctx->ball_y[ball_idx];
        
        // VULKAN ADVANTAGE: Variable ball sizes based on index
        // This creates more complex geometry that Vulkan handles better
        float size_multiplier = 1.0f + (ball_idx % 3) * 0.5f; // 1.0x, 1.5x, 2.0x sizes
        float current_radius = ball_radius * size_multiplier;
        
        // RECTANGLE OPTIMIZATION with variable sizes
        int x_start = (int)(ball_center_x - current_radius);
        int y_start = (int)(ball_center_y - current_radius);
        int width = (int)(current_radius * 2);
        int height = (int)(current_radius * 2);
        
        // Clamp to screen bounds
        if (x_start < 0) { width += x_start; x_start = 0; }
        if (y_start < 0) { height += y_start; y_start = 0; }
        if (x_start + width >= (int)ctx->swapchain_extent.width) 
            width = ctx->swapchain_extent.width - x_start;
        if (y_start + height >= (int)ctx->swapchain_extent.height) 
            height = ctx->swapchain_extent.height - y_start;
            
        if (width > 0 && height > 0) {
            // VULKAN ADVANTAGE: Multiple rectangles per ball for complex effects
            // Main ball rectangle
            VkClearRect ball_rect = {
                .rect = {
                    .offset = {x_start, y_start},
                    .extent = {width, height}
                },
                .baseArrayLayer = 0,
                .layerCount = 1
            };
            
            // Dynamic color based on ball index
            float red = 1.0f;
            float green = (ball_idx % 2) * 0.3f;
            float blue = (ball_idx % 3) * 0.2f;
            
            VkClearValue ball_color = {{red, green, blue, 1.0f}};
            VkClearAttachment color_attachment = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .colorAttachment = 0,
                .clearValue = ball_color
            };
            
            vkCmdClearAttachments(ctx->command_buffers[image_index], 1, &color_attachment, 1, &ball_rect);
            
            // VULKAN SPECIFIC: Add highlight rectangles (OpenGL version won't have this)
            // VULKAN ADVANTAGE: All clear operations are batched efficiently in command buffer
            if (ball_idx % 2 == 0 && current_radius > 25) {
                // Inner highlight rectangle
                int highlight_size = (int)(current_radius * 0.6f);
                VkClearRect highlight_rect = {
                    .rect = {
                        .offset = {(int)(ball_center_x - highlight_size/2), (int)(ball_center_y - highlight_size/2)},
                        .extent = {highlight_size, highlight_size}
                    },
                    .baseArrayLayer = 0,
                    .layerCount = 1
                };
                
                VkClearValue highlight_color = {{1.0f, 1.0f, 0.5f, 1.0f}}; // Yellow highlight
                VkClearAttachment highlight_attachment = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .colorAttachment = 0,
                    .clearValue = highlight_color
                };
                
                vkCmdClearAttachments(ctx->command_buffers[image_index], 1, &highlight_attachment, 1, &highlight_rect);
            }
            
            // VULKAN EFFICIENCY: Multiple operations batched without state changes
            // Unlike OpenGL, these don't cause pipeline stalls
            if (current_radius > 30) {
                for (int sub = 0; sub < 4; sub++) {
                    int sub_size = 3;
                    int sub_x = (int)ball_center_x + (sub % 2) * ((int)current_radius - sub_size);
                    int sub_y = (int)ball_center_y + (sub / 2) * ((int)current_radius - sub_size);
                    
                    if (sub_x >= 0 && sub_y >= 0 && 
                        sub_x + sub_size < (int)ctx->swapchain_extent.width &&
                        sub_y + sub_size < (int)ctx->swapchain_extent.height) {
                        
                        VkClearRect sub_rect = {
                            .rect = {
                                .offset = {sub_x, sub_y},
                                .extent = {sub_size, sub_size}
                            },
                            .baseArrayLayer = 0,
                            .layerCount = 1
                        };
                        
                        VkClearValue sub_color = {{0.0f, 0.0f, 1.0f, 1.0f}}; // Blue corner dots
                        VkClearAttachment sub_attachment = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .colorAttachment = 0,
                            .clearValue = sub_color
                        };
                        
                        vkCmdClearAttachments(ctx->command_buffers[image_index], 1, &sub_attachment, 1, &sub_rect);
                    }
                }
            }
        }
    }
    
    // Check if notification should be shown as overlay box (fixed 4-second cycle)
    int sec = ((int)ctx->time) % 4 + 1;
    bool show_notification = (sec == 3 || sec == 4);
    
    if (show_notification) {
        // Draw animated notification box split into segments based on number of balls
        float pulse = sin(ctx->time * 6.0f) * 0.5f + 0.5f; // Pulsing animation
        float size_mod = 1.0f + pulse * 0.1f; // Size pulsing
        uint32_t box_width = (uint32_t)(400 * size_mod);
        uint32_t box_height = (uint32_t)(150 * size_mod);
        uint32_t center_x = (ctx->swapchain_extent.width - box_width) / 2;
        uint32_t center_y = (ctx->swapchain_extent.height - box_height) / 2;
        
        // Animated orange color with pulsing intensity
        float intensity = 0.9f + pulse * 0.1f;
        float orange_r = intensity;
        float orange_g = 0.5f + pulse * 0.2f;
        float orange_b = 0.1f;
        
        // Draw segmented rectangles based on number of balls + additional complexity
        int num_balls = ctx->active_balls;
        if (num_balls < 1) num_balls = 1; // Minimum 1 segment
        
        // Calculate segment dimensions
        int gap_size = 4; // Gap between segments
        int total_gaps = (num_balls > 1) ? (num_balls - 1) * gap_size : 0;
        int segment_width = (box_width - total_gaps) / num_balls;
        
        // Draw each segment with additional sub-segments for complexity
        for (int i = 0; i < num_balls; i++) {
            int segment_x = center_x + i * (segment_width + gap_size);
            
            // Main segment
            VkClearRect segment_rect = {
                .rect = {
                    .offset = {segment_x, center_y},
                    .extent = {segment_width, box_height}
                },
                .baseArrayLayer = 0,
                .layerCount = 1
            };
            
            VkClearValue segment_color = {{orange_r, orange_g, orange_b, 1.0f}};
            vkCmdClearAttachments(ctx->command_buffers[image_index], 1,
                                &(VkClearAttachment){.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .colorAttachment = 0, .clearValue = segment_color},
                                1, &segment_rect);
            
            // Add sub-segments for complexity (Vulkan handles multiple clear ops better)
            if (segment_width > 20) {
                int sub_height = box_height / 4;
                for (int j = 0; j < 3; j++) {
                    VkClearRect sub_rect = {
                        .rect = {
                            .offset = {segment_x + 2, center_y + j * (sub_height + 2) + 10},
                            .extent = {segment_width - 4, sub_height}
                        },
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    };
                    
                    float sub_intensity = intensity * (0.7f + j * 0.1f);
                    VkClearValue sub_color = {{sub_intensity, orange_g * 0.8f, orange_b * 1.2f, 1.0f}};
                    vkCmdClearAttachments(ctx->command_buffers[image_index], 1,
                                        &(VkClearAttachment){.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .colorAttachment = 0, .clearValue = sub_color},
                                        1, &sub_rect);
                }
            }
        }
    }
    
    vkCmdEndRenderPass(ctx->command_buffers[image_index]);
    vkEndCommandBuffer(ctx->command_buffers[image_index]);
}

bool draw_frame(VulkanContext *ctx) {
    // Safety checks
    if (!ctx || !ctx->device || !ctx->command_buffers || !ctx->fence) {
        printf("❌ draw_frame: Invalid context or objects\n");
        return false;
    }
    
    vkWaitForFences(ctx->device, 1, &ctx->fence, VK_TRUE, UINT64_MAX);
    vkResetFences(ctx->device, 1, &ctx->fence);
    
    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(ctx->device, ctx->swapchain, UINT64_MAX,
                                           ctx->image_available, VK_NULL_HANDLE, &image_index);
    
    if (result != VK_SUCCESS) {
        printf("❌ vkAcquireNextImageKHR failed: %d\n", result);
        return false;
    }
    
    if (image_index >= ctx->swapchain_image_count) {
        printf("❌ Invalid image index: %d >= %d\n", image_index, ctx->swapchain_image_count);
        return false;
    }
    
    vkResetCommandBuffer(ctx->command_buffers[image_index], 0);
    record_command_buffer(ctx, image_index);
    
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &ctx->image_available,
        .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = &ctx->command_buffers[image_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &ctx->render_finished
    };
    
    if (vkQueueSubmit(ctx->graphics_queue, 1, &submit_info, ctx->fence) != VK_SUCCESS) {
        return false;
    }
    
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &ctx->render_finished,
        .swapchainCount = 1,
        .pSwapchains = &ctx->swapchain,
        .pImageIndices = &image_index
    };
    
    vkQueuePresentKHR(ctx->graphics_queue, &present_info);
    
    return true;
}

void run_main_loop(VulkanContext *ctx, int test_duration) {
    printf("Starting game simulation...\n");
    printf("Watch for popup notifications that appear and disappear\n");
    printf("Monitoring FPS and CPU usage every second\n\n");
    
    double start_time = get_time();
    int frame_count = 0;
    double last_fps_time = start_time;
    
    // Average tracking
    double total_fps = 0.0;
    double total_cpu = 0.0;
    int sample_count = 0;
    
    // Initialize CPU monitoring
    calculate_cpu_usage(); // First call to initialize
    
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(ctx->physical_device, &props);
    printf("GPU: %s\n", props.deviceName);
    printf("Resolution: %dx%d\n\n", ctx->swapchain_extent.width, ctx->swapchain_extent.height);
    
    printf("🟢 Starting frame rendering...\n");
   
    while (true) {
        // Update animation time
        ctx->time = get_time() - start_time;
        
        // Handle Wayland events
        wl_display_dispatch_pending(display);
        
        // Draw frame
        if (!draw_frame(ctx)) {
            printf("❌ draw_frame failed, exiting\n");
            break;
        }
        
        frame_count++;

        // Print FPS and CPU usage every second
        double current_time = get_time();
        if (current_time - last_fps_time >= 1.0) {
            double fps = frame_count / (current_time - last_fps_time);
            float cpu_usage = calculate_cpu_usage();
            
            printf("[%.1fs] FPS: %.1f | CPU: ", current_time - start_time, fps);
            if (cpu_usage >= 0) {
                printf("%.1f%%", cpu_usage);
                if (cpu_usage > 80.0f) {
                    printf(" (HIGH)");
                }
                total_cpu += cpu_usage;
                
                // Export to CSV
                append_csv(csv_filename, "Vulkan", ctx->swapchain_extent.width, 
                          ctx->swapchain_extent.height, ctx->active_balls, fps, cpu_usage);
            } else {
                printf("N/A");
                // Export to CSV with -1 for invalid CPU
                append_csv(csv_filename, "Vulkan", ctx->swapchain_extent.width, 
                          ctx->swapchain_extent.height, ctx->active_balls, fps, -1.0);
            }
            
            printf("\n");
            
            // Add to averages
            total_fps += fps;
            sample_count++;
            
            frame_count = 0;
            last_fps_time = current_time;
        }
        
        // Run for specified duration then exit
        if (current_time - start_time > (double)test_duration) {
            break;
        }
    }
    
    double total_time = get_time() - start_time;
    printf("\n=== Test Completed ===\n");
    printf("Total runtime: %.1f seconds\n", total_time);
    printf("GPU: %s\n", props.deviceName);
    
    if (sample_count > 0) {
        printf("Average FPS: %.1f\n", total_fps / sample_count);
        printf("Average CPU Usage: %.1f%%\n", total_cpu / sample_count);
    }
    printf("GPU Performance Test: COMPLETED\n");
}

void cleanup_vulkan(VulkanContext *ctx) {
    vkDeviceWaitIdle(ctx->device);
    
    if (ctx->fence) vkDestroyFence(ctx->device, ctx->fence, NULL);
    if (ctx->render_finished) vkDestroySemaphore(ctx->device, ctx->render_finished, NULL);
    if (ctx->image_available) vkDestroySemaphore(ctx->device, ctx->image_available, NULL);
    
    if (ctx->command_pool) vkDestroyCommandPool(ctx->device, ctx->command_pool, NULL);
    
    // Clean up vertex buffer
    if (ctx->vertex_buffer) vkDestroyBuffer(ctx->device, ctx->vertex_buffer, NULL);
    if (ctx->vertex_buffer_memory) vkFreeMemory(ctx->device, ctx->vertex_buffer_memory, NULL);
    
    if (ctx->framebuffers) {
        for (uint32_t i = 0; i < ctx->swapchain_image_count; i++) {
            vkDestroyFramebuffer(ctx->device, ctx->framebuffers[i], NULL);
        }
        free(ctx->framebuffers);
    }
    
    if (ctx->main_pipeline) vkDestroyPipeline(ctx->device, ctx->main_pipeline, NULL);
    if (ctx->main_pipeline_layout) vkDestroyPipelineLayout(ctx->device, ctx->main_pipeline_layout, NULL);
    if (ctx->overlay_pipeline_layout) vkDestroyPipelineLayout(ctx->device, ctx->overlay_pipeline_layout, NULL);
    if (ctx->render_pass) vkDestroyRenderPass(ctx->device, ctx->render_pass, NULL);
    
    if (ctx->swapchain_image_views) {
        for (uint32_t i = 0; i < ctx->swapchain_image_count; i++) {
            vkDestroyImageView(ctx->device, ctx->swapchain_image_views[i], NULL);
        }
        free(ctx->swapchain_image_views);
    }
    
    if (ctx->swapchain_images) free(ctx->swapchain_images);
    if (ctx->swapchain) vkDestroySwapchainKHR(ctx->device, ctx->swapchain, NULL);
    if (ctx->surface) vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);
    if (ctx->device) vkDestroyDevice(ctx->device, NULL);
    if (ctx->instance) vkDestroyInstance(ctx->instance, NULL);
}

//=============================================================================
// MAIN
//=============================================================================

int main(int argc, char **argv) {
    printf("=================================\n");
    printf("Game Overlay Notification Test\n");
    printf("=================================\n");
    printf("Simulates game with popup notifications (achievements, directions, etc.)\n\n");
    
    // Parse command line arguments
    int duration = 10; // default test duration in seconds
    uint32_t width = 1920;  // default width
    uint32_t height = 1080; // default height
    bool constant_balls = true; // keep ball count constant at 1
    
    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], "--duration", 10) == 0) {
            if (argv[i][10] == '=' && argv[i][11]) {
                duration = atoi(argv[i] + 11);
            } else if (i + 1 < argc) {
                duration = atoi(argv[++i]);
            }
        } else if (strncmp(argv[i], "--width", 7) == 0) {
            if (argv[i][7] == '=' && argv[i][8]) {
                width = atoi(argv[i] + 8);
            } else if (i + 1 < argc) {
                width = atoi(argv[++i]);
            }
        } else if (strncmp(argv[i], "--height", 8) == 0) {
            if (argv[i][8] == '=' && argv[i][9]) {
                height = atoi(argv[i] + 9);
            } else if (i + 1 < argc) {
                height = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "--no-vsync") == 0) {
	    vsync = false;
        } else if (strcmp(argv[i], "--no-constant") == 0) {
            constant_balls = false;
        } else if (strcmp(argv[i], "--csv") == 0 && i + 1 < argc) {
            strncpy(csv_filename, argv[++i], sizeof(csv_filename) - 1);
            csv_filename[sizeof(csv_filename) - 1] = '\0';
        }
    }
   
    WIDTH = width;
    HEIGHT = height; 
    printf("Test duration: %d seconds\n", duration);
    printf("Resolution: %dx%d\n", width, height);
    printf("Ball mode: %s\n", constant_balls ? "Constant (1 ball)" : "Increasing (stress test)");
    printf("CSV output: %s\n", csv_filename);
    printf("Notifications will appear during seconds 3-4, 7-8, 13-14, etc. (4-second cycle)\n\n");
    
    if (!init_wayland()) {
        return 1;
    }
    
    VulkanContext ctx = {0};
    ctx.user_duration = duration;
    ctx.user_width = width;
    ctx.user_height = height;
    ctx.constant_balls = constant_balls;
    
    // Initialize ball physics arrays
    for (int i = 0; i < MAX_BALLS; i++) {
        ctx.ball_x[i] = 0.0f;
        ctx.ball_y[i] = 0.0f;
        ctx.ball_vx[i] = 0.0f;
        ctx.ball_vy[i] = 0.0f;
    }
    ctx.active_balls = 0;
    if (!init_vulkan(&ctx)) {
        cleanup_wayland();
        return 1;
    }
    
    if (!create_swapchain(&ctx) ||
        !create_render_pass(&ctx) ||
        !create_graphics_pipelines(&ctx) ||
        !create_framebuffers(&ctx) ||
        !create_command_buffers(&ctx) ||
        !create_sync_objects(&ctx)) {
        cleanup_vulkan(&ctx);
        cleanup_wayland();
        return 1;
    }
    
    run_main_loop(&ctx, duration);
    
    cleanup_vulkan(&ctx);
    cleanup_wayland();
    
    printf("Test completed successfully\n");
    return 0;
}
