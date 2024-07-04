/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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
#include <stdio.h>
#include "wayland-egl.h"

struct wl_egl_window;

struct wl_egl_window * wl_egl_window_create(struct wl_surface *surface, int width, int height)
{
     printf("\nDUMMY %s\n", __FUNCTION__);
     return NULL;
}

void wl_egl_window_destroy(struct wl_egl_window *egl_window)
{
     printf("\nDUMMY %s\n", __FUNCTION__);
}    

void wl_egl_window_resize(struct wl_egl_window *egl_window, int width, int height, int dx, int dy)
{
     printf("\nDUMMY %s\n", __FUNCTION__);
}

