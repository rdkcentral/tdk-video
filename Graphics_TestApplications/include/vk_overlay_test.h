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
#ifndef OVERLAY_TEST_H
#define OVERLAY_TEST_H

#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

// Wayland globals
struct wl_display *display;
struct wl_registry *registry;
struct wl_compositor *compositor;
struct wl_shell *shell;
struct wl_surface *surface;
struct wl_shell_surface *shell_surface;

// Vulkan context
typedef struct {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    uint32_t graphics_family;
    
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImage *swapchain_images;
    VkImageView *swapchain_image_views;
    uint32_t swapchain_image_count;
    VkExtent2D swapchain_extent;
    VkFormat swapchain_format;
    
    VkRenderPass render_pass;
    VkFramebuffer *framebuffers;
    
    // Graphics pipeline for main red screen
    VkPipeline main_pipeline;
    VkPipelineLayout main_pipeline_layout;
    
    // Graphics pipeline for overlay
    VkPipeline overlay_pipeline;
    VkPipelineLayout overlay_pipeline_layout;
    
    // Command buffers
    VkCommandPool command_pool;
    VkCommandBuffer *command_buffers;
    
    // Vertex buffer
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    
    // Synchronization
    VkSemaphore image_available;
    VkSemaphore render_finished;
    VkFence fence;
    
    // Animation data
    float time;
    int user_duration; // user-specified overlay cycle duration
    uint32_t user_width;  // user-specified width
    uint32_t user_height; // user-specified height
    
    // Ball animation state - support multiple balls
    #define MAX_BALLS 100
    float ball_x[MAX_BALLS], ball_y[MAX_BALLS];     // Ball positions
    float ball_vx[MAX_BALLS], ball_vy[MAX_BALLS];   // Ball velocities
    int active_balls;                               // Number of active balls
    bool constant_balls;                            // Keep ball count constant at 1
    
} VulkanContext;

// Vertex data for overlay quad
typedef struct {
    float pos[2];
    float color[3];
} Vertex;

// Function declarations
bool init_wayland(void);
void cleanup_wayland(void);

bool init_vulkan(VulkanContext *ctx);
void cleanup_vulkan(VulkanContext *ctx);

bool create_swapchain(VulkanContext *ctx);
bool create_render_pass(VulkanContext *ctx);
bool create_graphics_pipelines(VulkanContext *ctx);
bool create_framebuffers(VulkanContext *ctx);
bool create_command_buffers(VulkanContext *ctx);
bool create_sync_objects(VulkanContext *ctx);

void record_command_buffer(VulkanContext *ctx, uint32_t image_index);
bool draw_frame(VulkanContext *ctx);

void run_main_loop(VulkanContext *ctx, int test_duration);

// Utility functions
double get_time(void);
const char* get_vertex_shader_source(void);
const char* get_fragment_shader_source(void);
const char* get_overlay_vertex_shader_source(void);
const char* get_overlay_fragment_shader_source(void);

VkShaderModule create_shader_module(VkDevice device, const char *source, size_t length);

#endif // OVERLAY_TEST_H
