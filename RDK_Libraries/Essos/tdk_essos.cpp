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

#include "essos.h"
#include <stdlib.h>
#include <stdio.h>

EssCtx* EssContextCreate()
{
   EssCtx *ctx= 0;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return ctx;
}

void EssContextDestroy( EssCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

const char *EssContextGetLastErrorDetail( EssCtx *ctx )
{
   const char *detail= 0;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return detail;
}

bool EssContextSupportWayland( EssCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return true;
}

bool EssContextSupportDirect( EssCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return false;
}

bool EssContextSetUseWayland( EssCtx *ctx, bool useWayland )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextGetUseWayland( EssCtx *ctx )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;      
}

bool EssContextSetUseDirect( EssCtx *ctx, bool useDirect )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return EssContextSetUseWayland( ctx, !useDirect );
}

bool EssContextGetUseDirect( EssCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return !EssContextGetUseWayland( ctx );
}

bool EssContextSetName( EssCtx *ctx, const char *name )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetEGLConfigAttributes( EssCtx *ctx, EGLint *attrs, EGLint size )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return false;
}

bool EssContextGetEGLConfigAttributes( EssCtx *ctx, EGLint **attrs, EGLint *size )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetEGLSurfaceAttributes( EssCtx *ctx, EGLint *attrs, EGLint size )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextGetEGLSurfaceAttributes( EssCtx *ctx, EGLint **attrs, EGLint *size )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetEGLContextAttributes( EssCtx *ctx, EGLint *attrs, EGLint size )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextGetEGLContextAttributes( EssCtx *ctx, EGLint **attrs, EGLint *size )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetDisplayMode( EssCtx *ctx, const char *mode )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetInitialWindowSize( EssCtx *ctx, int width, int height )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetSwapInterval( EssCtx *ctx, EGLint swapInterval )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextInit( EssCtx *ctx )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextGetEGLDisplayType( EssCtx *ctx, NativeDisplayType *displayType )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextCreateNativeWindow( EssCtx *ctx, int width, int height, NativeWindowType *nativeWindow )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextDestroyNativeWindow( EssCtx *ctx, NativeWindowType nativeWindow )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

void* EssContextGetWaylandDisplay( EssCtx *ctx )
{
   void *wldisplay= 0;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return wldisplay;
}

bool EssContextSetKeyListener( EssCtx *ctx, void *userData, EssKeyListener *listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetKeyAndMetadataListener( EssCtx *ctx, void *userData, EssKeyAndMetadataListener *listener, EssInputDeviceMetadata *metadata )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetPointerListener( EssCtx *ctx, void *userData, EssPointerListener *listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetTouchListener( EssCtx *ctx, void *userData, EssTouchListener *listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetSettingsListener( EssCtx *ctx, void *userData, EssSettingsListener *listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetTerminateListener( EssCtx *ctx, void *userData, EssTerminateListener *listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetGamepadConnectionListener( EssCtx *ctx, void *userData, EssGamepadConnectionListener *listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssGamepadSetEventListener( EssGamepad *gp, void *userData, EssGamepadEventListener *listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

const char *EssGamepadGetDeviceName( EssGamepad *gp )
{
   const char *name= 0;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return name;
}

unsigned int EssGamepadGetDriverVersion( EssGamepad *gp )
{
   unsigned int version= 0;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return version;
}

bool EssGamepadGetButtonMap( EssGamepad *gp, int *count, int *map )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssGamepadGetAxisMap( EssGamepad *gp, int *count, int *map )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssGamepadGetState( EssGamepad *gp, int *buttonState, int *axisState )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetKeyRepeatInitialDelay( EssCtx *ctx, int delay )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetKeyRepeatPeriod( EssCtx *ctx, int period )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextStart( EssCtx *ctx )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

void EssContextStop( EssCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

bool EssContextSetDisplaySize( EssCtx *ctx, int width, int height )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return false;
}

bool EssContextGetDisplaySize( EssCtx *ctx, int *width, int *height )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextGetDisplaySafeArea( EssCtx *ctx, int *x, int *y, int *width, int *height )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextSetWindowPosition( EssCtx *ctx, int x, int y )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool EssContextResizeWindow( EssCtx *ctx, int width, int height )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

void EssContextRunEventLoopOnce( EssCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

void EssContextUpdateDisplay( EssCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}
