# Makefile.am for graphics test applications

ACLOCAL_AMFLAGS = -Im4 --install

AM_CPPFLAGS = -g -Wall -std=c++0x -DGCC4_XXX \
              -fPIC ${COPTS} $(GST_CFLAGS) $(GST_CHECK_CFLAGS) -DUSE_GST1 $(MEDIAPIPELINE_FLAGS)

AM_CFLAGS = -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
              -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/glib-2.0 \
              -I${PKG_CONFIG_SYSROOT_DIR}/usr/lib/glib-2.0/include \
              -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/gstreamer-1.0 \
              -I${top_srcdir}/Graphics_TestApplications/include

AM_CXXFLAGS = -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
              -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/glib-2.0 \
              -I${PKG_CONFIG_SYSROOT_DIR}/usr/lib/glib-2.0/include \
              -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/gstreamer-1.0 \
	      -I${top_srcdir}/Graphics_TestApplications/include

# Common libraries
if TDKGRAPHICS
COMMON_LIBS = -lwayland-client -ltdk-EGL -ltdk-GLESv2 -lvulkan -ltdk-wayland-egl -lwesteros_simpleshell_client
else
COMMON_LIBS = -lwayland-client -lEGL -lGLESv2 -lvulkan -lwayland-egl -lwesteros_simpleshell_client
endif

# Programs to build
bin_PROGRAMS = tiles_benchmark vkmultithread oglmultithread motion_benchmark vkoverlay ogloverlay

# Tiles benchmark executable
tiles_benchmark_SOURCES = src/tiles_overlay.cpp src/overlay.cpp
tiles_benchmark_LDADD = $(COMMON_LIBS)

# Multi threading test - Vulkan
vkmultithread_SOURCES = src/vkmulti.cpp src/overlay.cpp
vkmultithread_LDADD = $(COMMON_LIBS)

# Multi threading test - OpenGL
oglmultithread_SOURCES = src/multiogl.cpp src/overlay.cpp
oglmultithread_LDADD = $(COMMON_LIBS)

# Motion benchmark test
motion_benchmark_SOURCES = src/sphere_benchmark_overlay.cpp src/overlay.cpp
motion_benchmark_LDADD = $(COMMON_LIBS)

# Overlay test - Vulkan
vkoverlay_SOURCES = src/vk_overlay_test.c
vkoverlay_LDADD = $(COMMON_LIBS) -lm

# Overlay test - OpenGL
ogloverlay_SOURCES = src/gl_overlay_test.c
ogloverlay_LDADD = $(COMMON_LIBS) -lm

# Essos validation test
bin_PROGRAMS += Essos_TDKTestApp
Essos_TDKTestApp_SOURCES = src/EssosValidationSuite.cpp
Essos_TDKTestApp_CXXFLAGS = ${AM_CXXFLAGS}
if HAVE_EGL
Essos_TDKTestApp_CXXFLAGS += ${EGL_CFLAGS}
endif
if TDKGRAPHICS
Essos_TDKTestApp_LDFLAGS = $(AM_FLAGS) -lessos -ltdk-GLESv2
else
Essos_TDKTestApp_LDFLAGS = $(AM_FLAGS) -lessos -lGLESv2
endif

# Westeros validation test
bin_PROGRAMS += Westeros_TDKTestApp
Westeros_TDKTestApp_SOURCES = src/WesterosValidationSuite.cpp
if TDKGRAPHICS
Westeros_TDKTestApp_LDFLAGS= \
   -lwayland-server \
   -lwayland-client \
   -lxkbcommon \
   -lwesteros_simpleshell_client \
   -ltdk-wayland-egl -ltdk-GLESv2\
   -ltdk-EGL
else
Westeros_TDKTestApp_LDFLAGS= \
   -lwayland-server \
   -lwayland-client \
   -lxkbcommon \
   -lwesteros_simpleshell_client \
   -lwayland-egl -lGLESv2\
   -lEGL
endif

# Include header files in distribution
noinst_HEADERS = include/overlay.h include/font8x8_basic.h include/vk_overlay_test.h
