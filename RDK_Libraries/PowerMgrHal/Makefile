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
INC_DIR = ${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rdk/iarmmgrs-hal
INC_DIR2 = ../../RDK_Source/iarmmgrs_source/hal/include
CXXFLAGS += -I$(INC_DIR) -I$(INC_DIR2)
CXXFLAGS += -std=c++1y  -g -fPIC -D_REENTRANT -Wall -DENABLE_THERMAL_PROTECTION

# Library name and version
LIBNAME = iarmmgrs-power-hal
LIBVERSION ?=  $(PWRMGR_HAL_LIB_VERSION)

# Source files
SRCS = tdk_power_hal.cpp

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
