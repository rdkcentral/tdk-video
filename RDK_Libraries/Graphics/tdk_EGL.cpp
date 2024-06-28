
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
#include "EGL/egl.h"

EGLBoolean eglSwapBuffers (EGLDisplay dpy, EGLSurface surface)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLBoolean eglMakeCurrent (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLBoolean eglDestroySurface (EGLDisplay dpy, EGLSurface surface)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLBoolean eglTerminate (EGLDisplay dpy)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLBoolean eglReleaseThread (void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLDisplay eglGetDisplay (EGLNativeDisplayType display_id)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return NULL;
}

EGLBoolean eglInitialize (EGLDisplay dpy, EGLint *major, EGLint *minor)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLBoolean eglGetConfigs (EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLint eglGetError (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return 1;
}

EGLBoolean eglChooseConfig (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLBoolean eglGetConfigAttrib (EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}

EGLContext eglCreateContext (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return NULL;
}

EGLSurface eglCreateWindowSurface (EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return NULL;
}

EGLBoolean eglSwapInterval (EGLDisplay dpy, EGLint interval)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}
