##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
INC_DIR = ${PKG_CONFIG_SYSROOT_DIR}${includedir}/
CXXFLAGS += -I$(INC_DIR)
CXXFLAGS += -std=c++1y -g -fPIC -D_REENTRANT -Wall

SRCS = tdk_GLESv2.cpp tdk_EGL.cpp tdk_wayland_egl.cpp

LIB_GLES = libtdk-GLESv2.so
LIB_EGL = libtdk-EGL.so
LIB_WAYLAND_EGL = libtdk-wayland-egl.so

all: $(LIB_GLES) $(LIB_EGL) $(LIB_WAYLAND_EGL)

$(LIB_GLES): tdk_GLESv2.cpp
	$(CXX) $(CXXFLAGS) -Wl,--hash-style=gnu -shared -Wl,-soname,$@.0 -o $@ $<
	ln -sf $@ $@.0
$(LIB_EGL): tdk_EGL.cpp
	$(CXX) $(CXXFLAGS) -Wl,--hash-style=gnu -shared -Wl,-soname,$@.0 -o $@ $<
	ln -sf $@ $@.0
$(LIB_WAYLAND_EGL): tdk_wayland_egl.cpp
	$(CXX) $(CXXFLAGS) -Wl,--hash-style=gnu -shared -Wl,-soname,$@.0 -o $@ $<
	ln -sf $@ $@.0

install: $(LIB_GLES) $(LIB_EGL) $(LIB_WAYLAND_EGL)
	@echo "Installing files in $(DESTDIR) ..."
	install -d $(DESTDIR)
	install -m 0755 $(LIB_GLES) $(DESTDIR)
	install -m 0755 $(LIB_GLES).0 $(DESTDIR)
	install -m 0755 $(LIB_EGL) $(DESTDIR)
	install -m 0755 $(LIB_EGL).0 $(DESTDIR)
	install -m 0755 $(LIB_WAYLAND_EGL) $(DESTDIR)
	install -m 0755 $(LIB_WAYLAND_EGL).0 $(DESTDIR)
clean:
	rm -f *.o

.PHONY: all clean install
