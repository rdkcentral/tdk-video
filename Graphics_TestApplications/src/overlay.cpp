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

#include "overlay.h"
#include "font8x8_basic.h"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <simpleshell-client-protocol.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <cstdio>
#include <cstring>

/* =========================================================
 * Globals (owned by tiles app)
 * ========================================================= */
wl_display*      gDisplay    = nullptr;
wl_compositor*   gCompositor = nullptr;
wl_simple_shell* gShell      = nullptr;

/* ---------------- Performance HUD ---------------- */
static wl_surface*    gPerfSurface = nullptr;
static wl_egl_window* gPerfWin     = nullptr;

static EGLDisplay gEglDisplay = EGL_NO_DISPLAY;
static EGLContext gPerfCtx    = EGL_NO_CONTEXT;
static EGLSurface gPerfSurf   = EGL_NO_SURFACE;

/* ---------------- Info HUD ---------------- */
static wl_surface*    gInfoSurface = nullptr;
static wl_egl_window* gInfoWin     = nullptr;

static EGLContext gInfoCtx  = EGL_NO_CONTEXT;
static EGLSurface gInfoSurf = EGL_NO_SURFACE;

/* ---------------- Sizes ---------------- */
int WINDOW_WIDTH  = 1920;
int WINDOW_HEIGHT = 1080;

int gOverlayW = 400;
int gOverlayH = 100;

int overlayWidth  = 300;
int overlayHeight = 100;

float uiScale = 1.0f;

/* =========================================================
 * Visual style (DESIGN ONLY)
 * ========================================================= */

static const float BG_MAIN[4]   = {0.06f, 0.07f, 0.09f, 0.88f};
static const float BG_HEADER[4] = {0.12f, 0.35f, 0.55f, 0.95f};

static const float TXT_MAIN[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
static const float TXT_SUB[4]   = {0.80f, 0.85f, 0.90f, 1.0f};

static const float BAR_BG[4]    = {0.22f, 0.24f, 0.28f, 1.0f};
static const float BAR_GOOD[4]  = {0.20f, 0.80f, 0.35f, 1.0f};
static const float BAR_WARN[4]  = {0.95f, 0.75f, 0.20f, 1.0f};
static const float BAR_BAD[4]   = {0.90f, 0.25f, 0.25f, 1.0f};

/* =========================================================
 * EGL save / restore (CRITICAL)
 * ========================================================= */

struct EGLSaved {
    EGLDisplay d;
    EGLSurface draw;
    EGLSurface read;
    EGLContext ctx;
};

static EGLSaved saveEGL()
{
    return {
        eglGetCurrentDisplay(),
        eglGetCurrentSurface(EGL_DRAW),
        eglGetCurrentSurface(EGL_READ),
        eglGetCurrentContext()
    };
}

static void restoreEGL(const EGLSaved& s)
{
    eglMakeCurrent(s.d, s.draw, s.read, s.ctx);
}
/* =========================================================
 * Shader (same as tiles_overlay.cpp)
 * ========================================================= */
static const char* hudVtx =
    "attribute vec2 pos;                 \n"
    "void main() {                       \n"
    "  gl_Position = vec4(pos,0.0,1.0);  \n"
    "}                                  \n";

static const char* hudFrag =
    "precision mediump float;            \n"
    "uniform vec4 color;                 \n"
    "void main() {                       \n"
    "  gl_FragColor = color;             \n"
    "}                                  \n";

static GLuint hudProg;
static GLint  hudPosLoc;
static GLint  hudColorLoc;

static GLuint compile(GLenum t, const char* s)
{
    GLuint sh = glCreateShader(t);
    glShaderSource(sh, 1, &s, nullptr);
    glCompileShader(sh);
    
    GLint success;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(sh, sizeof(infoLog), nullptr, infoLog);
        printf("[OVERLAY] Shader compilation error: %s\n", infoLog);
        glDeleteShader(sh);
        return 0;
    }
    
    return sh;
}

static void initHUDShader()
{
    GLuint vs = compile(GL_VERTEX_SHADER, hudVtx);
    GLuint fs = compile(GL_FRAGMENT_SHADER, hudFrag);
    
    if (vs == 0 || fs == 0) {
        printf("[OVERLAY] Failed to compile shaders\n");
        return;
    }

    hudProg = glCreateProgram();
    glAttachShader(hudProg, vs);
    glAttachShader(hudProg, fs);
    glLinkProgram(hudProg);
    
    GLint success;
    glGetProgramiv(hudProg, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(hudProg, sizeof(infoLog), nullptr, infoLog);
        printf("[OVERLAY] Program linking error: %s\n", infoLog);
        glDeleteProgram(hudProg);
        hudProg = 0;
        return;
    }
    
    // Clean up shader objects
    glDeleteShader(vs);
    glDeleteShader(fs);

    hudPosLoc   = glGetAttribLocation(hudProg, "pos");
    hudColorLoc = glGetUniformLocation(hudProg, "color");
}

/* =========================================================
 * Geometry helpers
 * ========================================================= */
static void hudRectSurf(float x, float y, float w, float h,
                        float sw, float sh,
                        float r, float g, float b, float a)
{
    float l   =  (2.0f * x) / sw - 1.0f;
    float rgt =  (2.0f * (x + w)) / sw - 1.0f;
    float t   =  1.0f - (2.0f * y) / sh;
    float btm =  1.0f - (2.0f * (y + h)) / sh;

    float v[] = {
        l,   t,
        rgt, t,
        l,   btm,
        rgt, btm
    };

    glUseProgram(hudProg);
    glUniform4f(hudColorLoc, r,g,b,a);
    glEnableVertexAttribArray(hudPosLoc);
    glVertexAttribPointer(hudPosLoc, 2, GL_FLOAT, GL_FALSE, 0, v);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


static void drawBarThreshold(
    int x, int y, int w, int h,
    float norm,        // 0..1
    float warnT,       // e.g. 0.6
    float goodT,       // e.g. 0.9
    float sw, float sh
) {
    // background
    hudRectSurf(x, y, w, h, sw, sh,
                BAR_BG[0], BAR_BG[1], BAR_BG[2], BAR_BG[3]);

    float r,g,b,a;
    if (norm >= goodT) {
        r=BAR_GOOD[0]; g=BAR_GOOD[1]; b=BAR_GOOD[2]; a=BAR_GOOD[3];
    } else if (norm >= warnT) {
        r=BAR_WARN[0]; g=BAR_WARN[1]; b=BAR_WARN[2]; a=BAR_WARN[3];
    } else {
        r=BAR_BAD[0]; g=BAR_BAD[1]; b=BAR_BAD[2]; a=BAR_BAD[3];
    }

    hudRectSurf(x, y, int(w * norm), h, sw, sh, r,g,b,a);
}

/* =========================================================
 * Font helpers
 * ========================================================= */
static void drawCharSurf(int x, int y, char c,
                         float sw, float sh,
                         float r, float g, float b, float a,
                         float scale)
{
    if (c < 32 || c > 127) return;
    const unsigned char* glyph = font8x8_basic[c - 32];

    for (int ry = 0; ry < 8; ry++) {
        for (int cx = 0; cx < 8; cx++) {
            if (glyph[ry] & (1 << cx)) {
                hudRectSurf(
                    x + cx * scale,
                    y + ry * scale,
                    scale, scale,
                    sw, sh,
                    r, g, b, a
                );
            }
        }
    }
}


static void drawTextSurf(int x, int y, const char* s,
                         float sw, float sh,
                         float r, float g, float b, float a,
                         float scale)
{
    for (; *s; ++s) {
        drawCharSurf(x, y, *s, sw, sh, r, g, b, a, scale);
        x += int(9 * scale);
    }
}

/* =========================================================
 * simple-shell listener
 * ========================================================= */
static void surface_id_cb(void*, wl_simple_shell*,
                          wl_surface* surf, uint32_t id)
{
    if (surf == gPerfSurface) {
        wl_simple_shell_set_name(gShell, id, "perf-hud");
        wl_simple_shell_set_geometry(gShell, id,
            20, 20, gOverlayW, gOverlayH);
        wl_simple_shell_set_zorder(gShell, id, wl_fixed_from_int(100));
        wl_simple_shell_set_opacity(gShell, id, wl_fixed_from_double(0.85));
        wl_simple_shell_set_visible(gShell, id, 1);
    }
    else if (surf == gInfoSurface) {
        wl_simple_shell_set_name(gShell, id, "info-hud");
        wl_simple_shell_set_geometry(
            gShell, id,
            WINDOW_WIDTH - overlayWidth - 20,
            20,
            overlayWidth, overlayHeight
        );
        wl_simple_shell_set_zorder(gShell, id, wl_fixed_from_int(90));
        wl_simple_shell_set_opacity(gShell, id, wl_fixed_from_double(0.85));
        wl_simple_shell_set_visible(gShell, id, 1);
    }
    wl_display_flush(gDisplay);
}

static void surface_created_cb(void*, wl_simple_shell*, uint32_t, const char*) {}
static void surface_destroyed_cb(void*, wl_simple_shell*, uint32_t, const char*) {}
static void surface_status_cb(void*, wl_simple_shell*, uint32_t,
                              const char*, uint32_t,
                              int32_t,int32_t,int32_t,int32_t,
                              wl_fixed_t, wl_fixed_t) {}
static void get_surfaces_done_cb(void*, wl_simple_shell*) {}

static const wl_simple_shell_listener gShellListener = {
    surface_id_cb,
    surface_created_cb,
    surface_destroyed_cb,
    surface_status_cb,
    get_surfaces_done_cb
};

/* =========================================================
 * Init overlays
 * ========================================================= */
void overlayInit(wl_display* d,
                 wl_compositor* c,
                 wl_simple_shell* s,
		 EGLDisplay sharedEglDisplay,
		 EGLContext       sharedCtx,
                 int winW,
                 int winH)
{
    gDisplay = d;
    gCompositor = c;
    gShell = s;

    WINDOW_WIDTH  = winW;
    WINDOW_HEIGHT = winH;

    uiScale = (float)winH / 1080.0f;
    if (uiScale < 1.0f) uiScale = 1.0f;
    if (uiScale > 2.0f) uiScale = 2.0f;

    gOverlayW = winW * 0.30f;
    gOverlayH = winH * 0.18f;
    overlayWidth  = winW * 0.24f;
    overlayHeight = winH * 0.12f;

    gPerfSurface = wl_compositor_create_surface(c);
    gInfoSurface = wl_compositor_create_surface(c);
    wl_simple_shell_add_listener(s, &gShellListener, nullptr);

    gPerfWin = wl_egl_window_create(gPerfSurface, gOverlayW, gOverlayH);
    gInfoWin = wl_egl_window_create(gInfoSurface, overlayWidth, overlayHeight);

    if (sharedEglDisplay == EGL_NO_DISPLAY) {
        gEglDisplay = eglGetDisplay((EGLNativeDisplayType)d);
        if (gEglDisplay == EGL_NO_DISPLAY) {
            printf("[OVERLAY] Failed to get EGL display\n");
            return;
        }
        if (!eglInitialize(gEglDisplay, nullptr, nullptr)) {
            printf("[OVERLAY] Failed to initialize EGL\n");
            return;
        }
    } else {
        gEglDisplay = sharedEglDisplay;
    }

    EGLConfig cfg; EGLint n;
    EGLint cfgAttr[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    if (!eglChooseConfig(gEglDisplay, cfgAttr, &cfg, 1, &n) || n == 0) {
        printf("[OVERLAY] Failed to choose EGL config\n");
        return;
    }

    EGLint ctxAttr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    gPerfCtx = eglCreateContext(gEglDisplay, cfg, sharedCtx, ctxAttr);
    if (gPerfCtx == EGL_NO_CONTEXT) {
        printf("[OVERLAY] Failed to create performance context\n");
        return;
    }
    
    gInfoCtx = eglCreateContext(gEglDisplay, cfg, gPerfCtx, ctxAttr);
    if (gInfoCtx == EGL_NO_CONTEXT) {
        printf("[OVERLAY] Failed to create info context\n");
        return;
    }

    gPerfSurf = eglCreateWindowSurface(
        gEglDisplay, cfg, (EGLNativeWindowType)gPerfWin, nullptr);
    if (gPerfSurf == EGL_NO_SURFACE) {
        printf("[OVERLAY] Failed to create performance surface\n");
        return;
    }
    
    gInfoSurf = eglCreateWindowSurface(
        gEglDisplay, cfg, (EGLNativeWindowType)gInfoWin, nullptr);
    if (gInfoSurf == EGL_NO_SURFACE) {
        printf("[OVERLAY] Failed to create info surface\n");
        return;
    }

    EGLSaved saved = saveEGL();
    if (!eglMakeCurrent(gEglDisplay, gPerfSurf, gPerfSurf, gPerfCtx)) {
        printf("[OVERLAY] Failed to make context current\n");
        restoreEGL(saved);
        return;
    }
    
    eglSwapInterval(gEglDisplay, 1);
    initHUDShader();
    if (hudProg == 0) {
        printf("[OVERLAY] Failed to initialize shader program\n");
    }
    restoreEGL(saved);
}

/* =========================================================
 * Draw performance HUD (UPGRADED)
 * ========================================================= */
void overlayUpdatePerf(double fps, double cpu)
{
    if (!gPerfCtx || !gPerfSurf || hudProg == 0) {
        return; // Not properly initialized
    }
    
    EGLSaved saved = saveEGL();
    eglMakeCurrent(gEglDisplay, gPerfSurf, gPerfSurf, gPerfCtx);

    glViewport(0,0,gOverlayW,gOverlayH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    float s = uiScale;
    int PAD   = int(14 * s);
    int HEAD  = int(26 * s);
    int LINE  = int(18 * s);
    int BAR_H = int(8  * s);

    // Background
    hudRectSurf(0,0,gOverlayW,gOverlayH,
                gOverlayW,gOverlayH,
                BG_MAIN[0],BG_MAIN[1],BG_MAIN[2],BG_MAIN[3]);

    // Header
    hudRectSurf(0,0,gOverlayW,HEAD,
                gOverlayW,gOverlayH,
                BG_HEADER[0],BG_HEADER[1],BG_HEADER[2],BG_HEADER[3]);

    drawTextSurf(PAD, int(6*s), "PERFORMANCE",
                 gOverlayW,gOverlayH,
                 TXT_MAIN[0],TXT_MAIN[1],TXT_MAIN[2],TXT_MAIN[3],
                 1.4f*s);

    char buf[64];
    int y = HEAD + PAD;

    // FPS
    snprintf(buf,sizeof(buf),"FPS  %.1f", fps);
    drawTextSurf(PAD, y, buf,
                 gOverlayW,gOverlayH,
                 TXT_MAIN[0],TXT_MAIN[1],TXT_MAIN[2],TXT_MAIN[3],
                 1.2f*s);

    drawBarThreshold(
        PAD, y + LINE,
        gOverlayW - PAD*2, BAR_H,
        (float)(fps / 60.0f),
        0.60f, 0.92f,
        gOverlayW, gOverlayH
    );

    // CPU
    y += LINE*2;
    snprintf(buf,sizeof(buf),"CPU  %.1f %%", cpu);
    drawTextSurf(PAD, y, buf,
                 gOverlayW,gOverlayH,
                 TXT_MAIN[0],TXT_MAIN[1],TXT_MAIN[2],TXT_MAIN[3],
                 1.2f*s);

    drawBarThreshold(
        PAD, y + LINE,
        gOverlayW - PAD*2, BAR_H,
        (float)(cpu / 100.0f),
        0.40f, 0.70f,
        gOverlayW, gOverlayH
    );

    eglSwapBuffers(gEglDisplay, gPerfSurf);
    wl_display_dispatch_pending(gDisplay);
    restoreEGL(saved);
}


/* =========================================================
 * Draw info HUD (UPGRADED)
 * ========================================================= */
void overlayShowInfo(const std::string& appname,
                     const std::string& backend,
                     int objects)
{
    if (!gInfoCtx || !gInfoSurf || hudProg == 0) {
        return; // Not properly initialized
    }
    
    EGLSaved saved = saveEGL();
    eglMakeCurrent(gEglDisplay, gInfoSurf, gInfoSurf, gInfoCtx);

    glViewport(0,0,overlayWidth,overlayHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    float s = uiScale;
    int PAD = int(14 * s);
    int LINE = int(18 * s);

    // Background
    hudRectSurf(0,0,overlayWidth,overlayHeight,
                overlayWidth,overlayHeight,
                BG_MAIN[0],BG_MAIN[1],BG_MAIN[2],BG_MAIN[3]);

    // Accent bar
    hudRectSurf(0,0,5,overlayHeight,
                overlayWidth,overlayHeight,
                BG_HEADER[0],BG_HEADER[1],BG_HEADER[2],1.0f);

    // Title
    drawTextSurf(PAD, PAD, appname.c_str(),
                 overlayWidth,overlayHeight,
                 TXT_MAIN[0],TXT_MAIN[1],TXT_MAIN[2],TXT_MAIN[3],
                 1.4f*s);

    // Backend
    drawTextSurf(PAD, PAD + LINE, backend.c_str(),
                 overlayWidth,overlayHeight,
                 TXT_SUB[0],TXT_SUB[1],TXT_SUB[2],TXT_SUB[3],
                 1.2f*s);

    char buf[64];
    int y = PAD + LINE*2;

    snprintf(buf,sizeof(buf),"RESOLUTION : %dx%d",
             WINDOW_WIDTH, WINDOW_HEIGHT);
    drawTextSurf(PAD, y, buf,
                 overlayWidth,overlayHeight,
                 TXT_SUB[0],TXT_SUB[1],TXT_SUB[2],TXT_SUB[3],
                 1.1f*s);

    y += LINE;
    snprintf(buf,sizeof(buf),"OBJECTS    : %d", objects);
    drawTextSurf(PAD, y, buf,
                 overlayWidth,overlayHeight,
                 TXT_SUB[0],TXT_SUB[1],TXT_SUB[2],TXT_SUB[3],
                 1.1f*s);

    eglSwapBuffers(gEglDisplay, gInfoSurf);
    restoreEGL(saved);
}


/* =========================================================
 * Shutdown
 * ========================================================= */
void overlayShutdown()
{
    EGLSaved saved = saveEGL();
    
    // Clean up shader program
    if (hudProg) {
        glDeleteProgram(hudProg);
        hudProg = 0;
    }
    
    eglMakeCurrent(gEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    
    if (gPerfSurf) {
        eglDestroySurface(gEglDisplay, gPerfSurf);
        gPerfSurf = EGL_NO_SURFACE;
    }
    if (gInfoSurf) {
        eglDestroySurface(gEglDisplay, gInfoSurf);
        gInfoSurf = EGL_NO_SURFACE;
    }
    if (gPerfCtx) {
        eglDestroyContext(gEglDisplay, gPerfCtx);
        gPerfCtx = EGL_NO_CONTEXT;
    }
    if (gInfoCtx) {
        eglDestroyContext(gEglDisplay, gInfoCtx);
        gInfoCtx = EGL_NO_CONTEXT;
    }
    
    restoreEGL(saved);
    
    if (gPerfWin) {
        wl_egl_window_destroy(gPerfWin);
        gPerfWin = nullptr;
    }
    if (gInfoWin) {
        wl_egl_window_destroy(gInfoWin);
        gInfoWin = nullptr;
    }
    if (gPerfSurface) {
        wl_surface_destroy(gPerfSurface);
        gPerfSurface = nullptr;
    }
    if (gInfoSurface) {
        wl_surface_destroy(gInfoSurface);
        gInfoSurface = nullptr;
    }
}
