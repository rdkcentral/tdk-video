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

#ifndef OVERLAY_H
#define OVERLAY_H

#include <string>

#include <wayland-client.h>
#include <simpleshell-client-protocol.h>

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize overlay HUD.
 *
 * For OpenGL spheres:
 *   - Pass the SAME EGLDisplay used by the main renderer.
 *
 * For Vulkan spheres:
 *   - Pass EGL_NO_DISPLAY
 *   - Overlay will create and manage its own EGLDisplay internally.
 *
 * Parameters:
 *   display     : wl_display from main app
 *   compositor  : wl_compositor from registry
 *   shell       : wl_simple_shell from registry
 *   sharedEgl   : EGLDisplay or EGL_NO_DISPLAY
 *   winW        : main window width
 *   winH        : main window height
 */
void overlayInit(
    wl_display*      display,
    wl_compositor*   compositor,
    wl_simple_shell* shell,
    EGLDisplay       sharedEgl,
	EGLContext       sharedCtx,
    int              winW,
    int              winH
);

/*
 * Update performance HUD (FPS / CPU).
 * Can be called once per second.
 */
void overlayUpdatePerf(double fps, double cpu);

/*
 * Show static info HUD (app name, backend, scene params).
 * Can be called once or occasionally.
 */
void overlayShowInfo(
    const std::string& appname,
    const std::string& backend,
    int                objects
);

/*
 * Destroy overlay resources.
 * Safe to call at shutdown.
 */
void overlayShutdown();

#ifdef __cplusplus
}
#endif

#endif // OVERLAY_H

