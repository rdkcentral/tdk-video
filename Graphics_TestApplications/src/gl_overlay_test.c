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
#include "gl_overlay_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stddef.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

bool vsync = true;

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
// Linux CPU usage calculation
static unsigned long long last_cpu_total = 0, last_cpu_idle = 0;

float calculate_cpu_usage(void) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return -1.0f;
    
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    if (fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq) != 7) {
        fclose(fp);
        return -1.0f;
    }
    fclose(fp);
    
    unsigned long long current_total = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long current_idle = idle + iowait;
    
    if (last_cpu_total == 0) {
        last_cpu_total = current_total;
        last_cpu_idle = current_idle;
        return 0.0f;
    }
    
    unsigned long long total_diff = current_total - last_cpu_total;
    unsigned long long idle_diff = current_idle - last_cpu_idle;
    
    float cpu_percentage = 0.0f;
    if (total_diff > 0) {
        cpu_percentage = ((float)(total_diff - idle_diff) / total_diff) * 100.0f;
    }
    
    last_cpu_total = current_total;
    last_cpu_idle = current_idle;
    
    return cpu_percentage;
}
#endif

//=============================================================================
// GPU INFO RETRIEVAL
//=============================================================================

const char* get_gpu_info(void) {
    return (const char*)glGetString(GL_RENDERER);
}

//=============================================================================
// TIMING UTILITIES
//=============================================================================

double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// CSV export functionality
char csv_filename[256] = "gl_overlay_test.csv"; // Default CSV filename

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

struct wl_display *display;
struct wl_registry *registry;
struct wl_compositor *compositor;
struct wl_shell *shell;
struct wl_surface *surface;
struct wl_shell_surface *shell_surface;

static void registry_global(void *data, struct wl_registry *registry, uint32_t id,
                           const char *interface, uint32_t version) {
    (void)data; (void)version;
    
    printf("Registry global: %s\n", interface);
    
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
        printf("Bound wl_compositor\n");
    } else if (strcmp(interface, "wl_shell") == 0) {
        shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
        printf("Bound wl_shell\n");
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
    // Handle global removal if needed
}

static const struct wl_registry_listener registry_listener = {
    registry_global,
    registry_global_remove
};

static void shell_surface_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial) {
    (void)data;
    wl_shell_surface_pong(shell_surface, serial);
}

static void shell_surface_configure(void *data, struct wl_shell_surface *shell_surface,
                                   uint32_t edges, int32_t width, int32_t height) {
    (void)data; (void)shell_surface; (void)edges; (void)width; (void)height;
}

static void shell_surface_popup_done(void *data, struct wl_shell_surface *shell_surface) {
    (void)data; (void)shell_surface;
}

static const struct wl_shell_surface_listener shell_surface_listener = {
    shell_surface_ping,
    shell_surface_configure,
    shell_surface_popup_done
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
    
    wl_display_roundtrip(display);
    
    if (!compositor || !shell) {
        printf("❌ Required Wayland protocols not available\n");
        return false;
    }
    
    surface = wl_compositor_create_surface(compositor);
    shell_surface = wl_shell_get_shell_surface(shell, surface);
    wl_shell_surface_add_listener(shell_surface, &shell_surface_listener, NULL);
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
    
    printf("✅ Wayland cleanup complete\n");
}

//=============================================================================
// OPENGL/EGL SETUP
//=============================================================================

bool init_opengl(OpenGLContext *ctx) {
    printf("🔄 Initializing OpenGL...\n");
    
    // Create EGL window
    ctx->egl_window = wl_egl_window_create(surface, ctx->window_width, ctx->window_height);
    if (!ctx->egl_window) {
        printf("❌ Failed to create EGL window\n");
        return false;
    }
    
    // Get EGL display
    ctx->egl_display = eglGetDisplay((EGLNativeDisplayType)display);
    if (ctx->egl_display == EGL_NO_DISPLAY) {
        printf("❌ Failed to get EGL display\n");
        return false;
    }
    
    // Initialize EGL
    if (!eglInitialize(ctx->egl_display, NULL, NULL)) {
        printf("❌ Failed to initialize EGL\n");
        return false;
    }
    
    // Choose EGL config
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    
    EGLint num_configs;
    if (!eglChooseConfig(ctx->egl_display, config_attribs, &ctx->egl_config, 1, &num_configs)) {
        printf("❌ Failed to choose EGL config\n");
        return false;
    }
    
    // Create EGL context
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    ctx->egl_context = eglCreateContext(ctx->egl_display, ctx->egl_config, EGL_NO_CONTEXT, context_attribs);
    if (ctx->egl_context == EGL_NO_CONTEXT) {
        printf("❌ Failed to create EGL context\n");
        return false;
    }
    
    // Create EGL surface
    ctx->egl_surface = eglCreateWindowSurface(ctx->egl_display, ctx->egl_config,
                                             (EGLNativeWindowType)ctx->egl_window, NULL);
    if (ctx->egl_surface == EGL_NO_SURFACE) {
        printf("❌ Failed to create EGL surface\n");
        return false;
    }
    
    // Make context current
    if (!eglMakeCurrent(ctx->egl_display, ctx->egl_surface, ctx->egl_surface, ctx->egl_context)) {
        printf("❌ Failed to make EGL context current\n");
        return false;
    }
    
    EGLBoolean ok = eglSwapInterval(ctx->egl_display, vsync ? 1 : 0);
    
    printf("✅ OpenGL initialized successfully\n");
    return true;
}

void cleanup_opengl(OpenGLContext *ctx) {
    if (ctx->egl_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        
        if (ctx->egl_context != EGL_NO_CONTEXT) {
            eglDestroyContext(ctx->egl_display, ctx->egl_context);
        }
        
        if (ctx->egl_surface != EGL_NO_SURFACE) {
            eglDestroySurface(ctx->egl_display, ctx->egl_surface);
        }
        
        eglTerminate(ctx->egl_display);
    }
    
    if (ctx->egl_window) {
        wl_egl_window_destroy(ctx->egl_window);
    }
    
    printf("✅ OpenGL cleanup complete\n");
}

//=============================================================================
// BALL RENDERING
//=============================================================================

void render_frame(OpenGLContext *ctx) {
    float time = ctx->time;
    float dt = 1.0f / 60.0f; // Assume 60 FPS for physics
    
    // Calculate number of active balls
    int target_balls;
    if (ctx->constant_balls) {
        // Constant mode: keep exactly 1 ball throughout
        target_balls = 1;
    } else {
        // Stress test mode: 1 initially, +3 every 1.5 seconds
        target_balls = 1 + (int)(time / 1.5f) * 3;
        if (target_balls > MAX_BALLS) target_balls = MAX_BALLS;
    }
    
    // Initialize new balls when count increases
    if (target_balls > ctx->active_balls) {
        for (int i = ctx->active_balls; i < target_balls; i++) {
            // Start new balls at random positions
            ctx->ball_x[i] = 50.0f + (rand() % (ctx->window_width - 100));
            ctx->ball_y[i] = 50.0f + (rand() % (ctx->window_height - 100));
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
        ctx->ball_x[0] = ctx->window_width / 2.0f;
        ctx->ball_y[0] = ctx->window_height / 2.0f;
        ctx->ball_vx[0] = 200.0f;
        ctx->ball_vy[0] = 150.0f;
        ctx->active_balls = 1;
    }
    
    // Ball radius
    float ball_radius = 20.0f;
    
    // Update all active balls (with variable ball sizes)
    for (int i = 0; i < ctx->active_balls; i++) {
        // Variable radius for physics (matches rendering)
        float size_multiplier = 1.0f + (i % 3) * 0.5f;
        float current_radius = 20.0f * size_multiplier;
        
        // Update position
        ctx->ball_x[i] += ctx->ball_vx[i] * dt;
        ctx->ball_y[i] += ctx->ball_vy[i] * dt;
        
        // Bounce off walls (using variable radius)
        if (ctx->ball_x[i] - current_radius <= 0 || ctx->ball_x[i] + current_radius >= ctx->window_width) {
            ctx->ball_vx[i] = -ctx->ball_vx[i];
            ctx->ball_x[i] = ctx->ball_x[i] - current_radius <= 0 ? current_radius : ctx->window_width - current_radius;
        }
        if (ctx->ball_y[i] - current_radius <= 0 || ctx->ball_y[i] + current_radius >= ctx->window_height) {
            ctx->ball_vy[i] = -ctx->ball_vy[i];
            ctx->ball_y[i] = ctx->ball_y[i] - current_radius <= 0 ? current_radius : ctx->window_height - current_radius;
        }
    }
    
    // Set viewport and clear to white background
    glViewport(0, 0, ctx->window_width, ctx->window_height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Enable scissor test for precise pixel drawing
    glEnable(GL_SCISSOR_TEST);
    
    // Draw all active balls with variable sizes and colors (matches Vulkan)
    for (int ball_idx = 0; ball_idx < ctx->active_balls; ball_idx++) {
        float ball_center_x = ctx->ball_x[ball_idx];
        float ball_center_y = ctx->ball_y[ball_idx];
        
        // Variable ball sizes based on index (same as Vulkan)
        float size_multiplier = 1.0f + (ball_idx % 3) * 0.5f; // 1.0x, 1.5x, 2.0x sizes
        float ball_radius = 20.0f * size_multiplier;
        
        // Convert to OpenGL coordinates (flip Y)
        float gl_center_y = ctx->window_height - ball_center_y;
        
        // OPTIMIZED: Draw ball as single rectangle instead of circle
        // This matches the super-optimized Vulkan version for fair comparison
        int x_start = (int)(ball_center_x - ball_radius);
        int y_start = (int)(gl_center_y - ball_radius);
        int width = (int)(ball_radius * 2);
        int height = (int)(ball_radius * 2);
        
        // Clamp to screen bounds
        if (x_start < 0) { width += x_start; x_start = 0; }
        if (y_start < 0) { height += y_start; y_start = 0; }
        if (x_start + width >= (int)ctx->window_width) 
            width = ctx->window_width - x_start;
        if (y_start + height >= (int)ctx->window_height) 
            height = ctx->window_height - y_start;
            
        if (width > 0 && height > 0) {
            // Main ball rectangle with dynamic colors (same as Vulkan)
            float red = 1.0f;
            float green = (ball_idx % 2) * 0.3f;
            float blue = (ball_idx % 3) * 0.2f;
            
            glScissor(x_start, y_start, width, height);
            glClearColor(red, green, blue, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            // Add highlight rectangles (same as Vulkan for fair comparison)
            if (ball_idx % 2 == 0 && ball_radius > 25) {
                // Inner highlight rectangle
                int highlight_size = (int)(ball_radius * 0.6f);
                int highlight_x = (int)(ball_center_x - highlight_size/2);
                int highlight_y = (int)(gl_center_y - highlight_size/2);
                
                // Clamp highlight to screen bounds
                if (highlight_x >= 0 && highlight_y >= 0 && 
                    highlight_x + highlight_size < (int)ctx->window_width &&
                    highlight_y + highlight_size < (int)ctx->window_height) {
                    
                    glScissor(highlight_x, highlight_y, highlight_size, highlight_size);
                    glClearColor(1.0f, 1.0f, 0.5f, 1.0f); // Yellow highlight
                    glClear(GL_COLOR_BUFFER_BIT);
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
        int box_width = (int)(400 * size_mod);
        int box_height = (int)(150 * size_mod);
        int center_x = (ctx->window_width - box_width) / 2;
        int center_y = (ctx->window_height - box_height) / 2;
        
        // Convert to OpenGL coordinates
        int gl_center_y = ctx->window_height - center_y - box_height;
        
        // Animated orange color with pulsing intensity
        float intensity = 0.9f + pulse * 0.1f;
        float orange_r = intensity;
        float orange_g = 0.5f + pulse * 0.2f;
        float orange_b = 0.1f;
        
        // Draw segmented rectangles based on number of balls
        int num_balls = ctx->active_balls;
        if (num_balls < 1) num_balls = 1; // Minimum 1 segment
        
        // Calculate segment dimensions
        int gap_size = 4; // Gap between segments
        int total_gaps = (num_balls > 1) ? (num_balls - 1) * gap_size : 0;
        int segment_width = (box_width - total_gaps) / num_balls;
        
        // Draw each segment
        for (int i = 0; i < num_balls; i++) {
            int segment_x = center_x + i * (segment_width + gap_size);
            
            glScissor(segment_x, gl_center_y, segment_width, box_height);
            glClearColor(orange_r, orange_g, orange_b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }
    
    glDisable(GL_SCISSOR_TEST);
    
    // Swap buffers
    eglSwapBuffers(ctx->egl_display, ctx->egl_surface);
}

//=============================================================================
// MAIN LOOP
//=============================================================================

void run_main_loop(OpenGLContext *ctx, int test_duration) {
    printf("Starting game simulation...\n");
    printf("Watch for popup notifications that appear and disappear\n");
    printf("Monitoring FPS and CPU usage every second\n\n");
    
    const char* gpu_name = get_gpu_info();
    printf("GPU: %s\n", gpu_name ? gpu_name : "Unknown");
    printf("Resolution: %dx%d\n\n", ctx->window_width, ctx->window_height);
    
    printf("🟢 Starting frame rendering...\n");
    
    double start_time = get_time();
    double last_fps_time = start_time;
    double last_cpu_time = start_time;
    int frame_count = 0;
    int last_frame_count = 0;
    
    // Prime the CPU calculation  
    calculate_cpu_usage();
    usleep(100000); // 100ms
    calculate_cpu_usage();
    
    float total_fps = 0.0f;
    float total_cpu = 0.0f;
    int fps_samples = 0;
    
    while (true) {
        double current_time = get_time();
        ctx->time = (float)(current_time - start_time);
        
        // Check if test duration is reached
        if (ctx->time >= test_duration) {
            break;
        }
        
        // Update display
        wl_display_dispatch_pending(display);
        
        // Render frame
        render_frame(ctx);
        frame_count++;
        
        // Calculate and display FPS and CPU every second
        if (current_time - last_fps_time >= 1.0) {
            double elapsed = current_time - last_fps_time;
            int frames_rendered = frame_count - last_frame_count;
            float fps = frames_rendered / elapsed;
            
            float cpu_usage = calculate_cpu_usage();
            if (cpu_usage < 0) cpu_usage = 0; // Handle error case
            
            // Export to CSV
            append_csv(csv_filename, "OpenGL", ctx->window_width, 
                      ctx->window_height, ctx->active_balls, fps, cpu_usage);
            
            // Accumulate for average calculation
            total_fps += fps;
            total_cpu += cpu_usage;
            fps_samples++;
            
            const char* cpu_status = "";
            if (cpu_usage > 80.0f) cpu_status = " (HIGH)";
            else if (cpu_usage > 60.0f) cpu_status = " (MODERATE)";
            
            printf("[%.1fs] FPS: %.1f | CPU: %.1f%%%s\n", 
                   ctx->time, fps, cpu_usage, cpu_status);
            
            last_fps_time = current_time;
            last_frame_count = frame_count;
        }
    }
    
    printf("\n=== Test Completed ===\n");
    printf("Total runtime: %.1f seconds\n", ctx->time);
    printf("GPU: %s\n", gpu_name ? gpu_name : "Unknown");
    
    if (fps_samples > 0) {
        printf("Average FPS: %.1f\n", total_fps / fps_samples);
        printf("Average CPU Usage: %.1f%%\n", total_cpu / fps_samples);
    }
    
    printf("GPU Performance Test: COMPLETED\n\n");
}

//=============================================================================
// MAIN FUNCTION
//=============================================================================

int main(int argc, char **argv) {
    printf("=================================\n");
    printf("OpenGL Game Overlay Test\n");
    printf("=================================\n");
    printf("Simulates game with popup notifications (achievements, directions, etc.)\n\n");
    
    // Parse command line arguments
    int duration = 10; // default test duration in seconds
    uint32_t width = 1920;  // default width
    uint32_t height = 1080; // default height
    bool constant_balls = false; // keep ball count constant at 1
    
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
    
    printf("Test duration: %d seconds\n", duration);
    printf("Resolution: %dx%d\n", width, height);
    printf("Ball mode: %s\n", constant_balls ? "Constant (1 ball)" : "Increasing (stress test)");
    printf("CSV output: %s\n", csv_filename);
    printf("Notifications will appear during seconds 3-4, 7-8, 13-14, etc. (4-second cycle)\n\n");
    
    printf("🔄 Initializing Wayland...\n");
    if (!init_wayland()) {
        printf("❌ Failed to initialize Wayland\n");
        return 1;
    }
    printf("✅ Wayland initialized successfully\n");
    
    printf("🔄 Creating OpenGL context...\n");
    OpenGLContext ctx = {0};
    ctx.window_width = width;
    ctx.window_height = height;
    ctx.constant_balls = constant_balls;
    
    // Initialize ball physics arrays
    for (int i = 0; i < MAX_BALLS; i++) {
        ctx.ball_x[i] = 0.0f;
        ctx.ball_y[i] = 0.0f;
        ctx.ball_vx[i] = 0.0f;
        ctx.ball_vy[i] = 0.0f;
    }
    ctx.active_balls = 0;
    printf("✅ Context initialized\n");
    
    if (!init_opengl(&ctx)) {
        printf("❌ Failed to initialize OpenGL\n");
        cleanup_wayland();
        return 1;
    }
    
    run_main_loop(&ctx, duration);
    
    cleanup_opengl(&ctx);
    cleanup_wayland();
    
    printf("Test completed successfully\n");
    return 0;
}
