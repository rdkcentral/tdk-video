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

#include "westeros-gl.h"

typedef struct _WstGLCtx 
{
   bool secureGraphics;
} WstGLCtx;

static bool useSecureGraphics( void )
{
   bool useSecure= false;
   return useSecure;
}

WstGLCtx* WstGLInit()
{
   WstGLCtx *ctx= 0;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return ctx;
}

void WstGLTerm( WstGLCtx *ctx )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
}

#if defined(__cplusplus)
extern "C"
{
#endif
bool _WstGLGetDisplayInfo( WstGLCtx *ctx, WstGLDisplayInfo *displayInfo )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return WstGLGetDisplayInfo( ctx, displayInfo );
}

bool _WstGLGetDisplaySafeArea( WstGLCtx *ctx, int *x, int *y, int *w, int *h )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return WstGLGetDisplaySafeArea( ctx, x, y, w, h );
}

bool _WstGLAddDisplaySizeListener( WstGLCtx *ctx, void *userData, WstGLDisplaySizeCallback listener )
{
   printf("\nDUMMY %s\n", __FUNCTION__);   
   return WstGLAddDisplaySizeListener( ctx, userData, listener );
}

bool _WstGLRemoveDisplaySizeListener( WstGLCtx *ctx, WstGLDisplaySizeCallback listener )
{
   printf("\nDUMMY %s\n", __FUNCTION__);
   return WstGLRemoveDisplaySizeListener( ctx, listener );
}
#if defined(__cplusplus)
}
#endif

bool WstGLGetDisplayInfo( WstGLCtx *ctx, WstGLDisplayInfo *displayInfo )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool WstGLGetDisplaySafeArea( WstGLCtx *ctx, int *x, int *y, int *w, int *h )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool WstGLAddDisplaySizeListener( WstGLCtx *ctx, void *userData, WstGLDisplaySizeCallback listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

bool WstGLRemoveDisplaySizeListener( WstGLCtx *ctx, WstGLDisplaySizeCallback listener )
{
   bool result= false;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return result;
}

/*
 * WstGLCreateNativeWindow
 * Create a native window suitable for use as an EGLNativeWindow
 */
void* WstGLCreateNativeWindow( WstGLCtx *ctx, int x, int y, int width, int height )
{
   void *nativeWindow= 0;
   printf("\nDUMMY %s\n", __FUNCTION__);
   return nativeWindow;   
}

/*
 * WstGLDestroyNativeWindow
 * Destroy a native window created by WstGLCreateNativeWindow
 */
void WstGLDestroyNativeWindow( WstGLCtx *ctx, void *nativeWindow )
{
   if ( ctx )
   {
	printf("\nDUMMY %s\n", __FUNCTION__);
   }
}
