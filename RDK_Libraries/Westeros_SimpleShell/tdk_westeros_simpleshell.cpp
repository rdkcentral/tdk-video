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
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "westeros-simpleshell.h"
#include "wayland-server.h"
#include "wayland-util.h"
#include "simpleshell-server-protocol.h"

static void destroy_shell(struct wl_resource *resource);
static void wstSimpleShellBroadcastCreation( struct wl_simple_shell *shell, uint32_t surfaceId );

typedef struct _ShellInfo
{
   struct wl_client *client;
} ShellInfo;

typedef struct _PendingBroadcastInfo
{
   long long creationTime;
} PendingBroadcastInfo;

struct wl_simple_shell 
{
   void *userData;
};

static void wstISimpleShellSetName(struct wl_client *client, struct wl_resource *resource, 
                                      uint32_t surfaceId, const char *name);
static void wstISimpleShellSetVisible(struct wl_client *client, struct wl_resource *resource, 
                                      uint32_t surfaceId, uint32_t visible);
static void wstISimpleShellSetGeometry(struct wl_client *client, struct wl_resource *resource,
                                       uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height);
static void wstISimpleShellSetOpacity(struct wl_client *client, struct wl_resource *resource, 
                                      uint32_t surfaceId, wl_fixed_t opacity);
static void wstISimpleShellSetZOrder(struct wl_client *client, struct wl_resource *resource, 
                                     uint32_t surfaceId, wl_fixed_t zorder);
static void wstISimpleShellGetStatus(struct wl_client *client, struct wl_resource *resource, uint32_t surface);
static void wstISimpleShellGetSurfaces(struct wl_client *client, struct wl_resource *resource);
static void wstISimpleShellSetFocus(struct wl_client *client, struct wl_resource *resource,
                                     uint32_t surfaceId);
static void wstISimpleShellSetScale(struct wl_client *client, struct wl_resource *resource,
                                     uint32_t surfaceId, wl_fixed_t scaleX, wl_fixed_t scaleY);

const static struct wl_simple_shell_interface simple_shell_interface = {
   wstISimpleShellSetName,
   wstISimpleShellSetVisible,
   wstISimpleShellSetGeometry,
   wstISimpleShellSetOpacity,
   wstISimpleShellSetZOrder,
   wstISimpleShellGetStatus,
   wstISimpleShellGetSurfaces,
   wstISimpleShellSetFocus,
   wstISimpleShellSetScale
};

static void wstSimpleShellBroadcastSurfaceUpdate(struct wl_client *client, struct wl_simple_shell *shell, uint32_t surfaceId )
{
   const char *name= 0;
}

static void wstISimpleShellSetName(struct wl_client *client, struct wl_resource *resource, 
                                      uint32_t surfaceId, const char *name)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellSetVisible(struct wl_client *client, struct wl_resource *resource, 
                                      uint32_t surfaceId, uint32_t visible)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellSetGeometry(struct wl_client *client, struct wl_resource *resource, 
                                       uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellSetOpacity(struct wl_client *client, struct wl_resource *resource, 
                                      uint32_t surfaceId, wl_fixed_t opacity)
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellSetZOrder(struct wl_client *client, struct wl_resource *resource, 
                                     uint32_t surfaceId, wl_fixed_t zorder)
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellGetStatus(struct wl_client *client, struct wl_resource *resource, uint32_t surfaceId )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellGetSurfaces(struct wl_client *client, struct wl_resource *resource)
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellSetFocus(struct wl_client *client, struct wl_resource *resource,
                                    uint32_t surfaceId)
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstISimpleShellSetScale(struct wl_client *client, struct wl_resource *resource,
                                     uint32_t surfaceId, wl_fixed_t scaleX, wl_fixed_t scaleY)
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void destroy_shell(struct wl_resource *resource)
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstSimpleShellBind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static void wstSimpleShellBroadcastCreation( struct wl_simple_shell *shell, uint32_t surfaceId )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

static int wstSimpleShellTimeOut( void *data )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return 0;   
}

wl_simple_shell* WstSimpleShellInit( struct wl_display *display,
                                     wayland_simple_shell_callbacks *callbacks, 
                                     void *userData )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   struct wl_simple_shell *shell= 0;
   return shell;
}

void WstSimpleShellUninit( wl_simple_shell *shell )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

void WstSimpleShellNotifySurfaceCreated( wl_simple_shell *shell, struct wl_client *client, 
                                         struct wl_resource *surface_resource, uint32_t surfaceId )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

void WstSimpleShellNotifySurfaceDestroyed( wl_simple_shell *shell, struct wl_client *client, uint32_t surfaceId )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
        &wl_surface_interface,
};

static const struct wl_message wl_simple_shell_requests[] = {
        { "set_name", "us", types + 0 },
};

static const struct wl_message wl_simple_shell_events[] = {
        { "surface_id", "ou", types + 9 },
};

WL_EXPORT const struct wl_interface wl_simple_shell_interface = {
        "wl_simple_shell", 0,
        0, wl_simple_shell_requests,
        0, wl_simple_shell_events,
};

