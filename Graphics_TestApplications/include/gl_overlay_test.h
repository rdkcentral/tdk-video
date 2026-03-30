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
#ifndef GL_OVERLAY_TEST_H
#define GL_OVERLAY_TEST_H

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define MAX_BALLS 100

// OpenGL context
typedef struct {
    // Window dimensions
    uint32_t window_width;
    uint32_t window_height;
    
    // EGL/OpenGL objects
    struct wl_egl_window *egl_window;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;
    
    // Animation data
    float time;
    
    // Ball animation state - support multiple balls
    float ball_x[MAX_BALLS], ball_y[MAX_BALLS];     // Ball positions
    float ball_vx[MAX_BALLS], ball_vy[MAX_BALLS];   // Ball velocities
    int active_balls;                               // Number of active balls
    bool constant_balls;                            // Keep ball count constant at 1
    
} OpenGLContext;

// Function declarations
bool init_wayland(void);
void cleanup_wayland(void);

bool init_opengl(OpenGLContext *ctx);
void cleanup_opengl(OpenGLContext *ctx);

void render_frame(OpenGLContext *ctx);
void run_main_loop(OpenGLContext *ctx, int test_duration);

// Utility functions
double get_time(void);
const char* get_gpu_info(void);
float calculate_cpu_usage(void);

#endif // GL_OVERLAY_TEST_H
