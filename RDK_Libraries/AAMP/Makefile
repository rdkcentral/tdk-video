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
INC_DIR1 = ../../RDK_Source/aamp_source/
CXXFLAGS += -I$(INC_DIR1)
INC_DIR2 = ../../RDK_Source/aamp_source/subtitle/
CXXFLAGS += -I$(INC_DIR2)
INC_DIR3 = ../../RDK_Source/aampabr_source/
CXXFLAGS += -I$(INC_DIR3)
INC_DIR4 = ../../RDK_Source/aamp_source/drm/helper/
CXXFLAGS += -I$(INC_DIR4)
INC_DIR5 = ../../RDK_Source/aamp_source/drm/
CXXFLAGS += -I$(INC_DIR5)
INC_DIR6 = ../../RDK_Source/aampmetrics_source/
CXXFLAGS += -I$(INC_DIR6)
INC_DIR7 = ../../RDK_Source/aamp_source/downloader/
CXXFLAGS += -I$(INC_DIR7)
INC_DIR8 = ../../RDK_Source/aamp_source/closedcaptions/
CXXFLAGS += -I$(INC_DIR8)
INC_DIR9 = ../../RDK_Source/aamp_source/tsb/api/
CXXFLAGS += -I$(INC_DIR9)
CXXFLAGS += -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)
CXXFLAGS += -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/glib-2.0/
CXXFLAGS += -I${PKG_CONFIG_SYSROOT_DIR}/usr/lib/glib-2.0/include/
CXXFLAGS += -std=c++1y  -g -fPIC -D_REENTRANT -Wall 

# Library name and version
LIBNAME = aamp
LIBVERSION ?=  $(AAMP_LIB_VERSION)

# Source files
SRCS = tdk_aamp.cpp

# Output library filename
LIBRARY = lib$(LIBNAME).so

all: $(LIBRARY)

$(LIBRARY): $(SRCS)
ifneq ($(LIBVERSION),)
	$(CC) $(CXXFLAGS) -Wl,--hash-style=gnu -shared -Wl,-soname,lib$(LIBNAME).so.$(LIBVERSION) -o $(LIBRARY) $(SRCS)
	ln -sf $(LIBRARY) lib$(LIBNAME).so.$(LIBVERSION)
else
	$(CC) $(CXXFLAGS) -Wl,--hash-style=gnu -shared -Wl,-soname,$(LIBRARY) -o $(LIBRARY) $(SRCS)
endif

install: $(LIBRARY)
	@echo "Installing files in $(DESTDIR) ..."
	install -d $(DESTDIR)
	install -m 0755 $< $(DESTDIR)

clean:
	rm -f *.o

.PHONY: all clean
