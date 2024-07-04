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
INC_DIR = ../../RDK_Source/westeros_source/essos/
CXXFLAGS += -I$(INC_DIR)
CXXFLAGS += -std=c++1y  -g -fPIC -D_REENTRANT -Wall 

# Library name and version
LIBNAME = essos
LIBVERSION ?=  $(ESSOS_LIB_VERSION)

# Source files
SRCS = tdk_essos.cpp

# Output library filename
LIBRARY = lib$(LIBNAME).so

#Additional headers
lib$(LIBNAME)_la_include_HEADERS = essos.h essos-app.h essos-system.h essos-game.h

all: $(LIBRARY)

$(LIBRARY): $(SRCS)
ifeq ($(LIBVERSION),)
	$(CC) $(CXXFLAGS) $(LDFLAGS) -O1 -Wl,--hash-style=gnu -shared -Wl,-soname,$(LIBRARY) -o $(LIBRARY) $(SRCS)
else
	$(CC) $(CXXFLAGS) $(LDFLAGS) -O1 -Wl,--hash-style=gnu -shared -Wl,-soname,lib$(LIBNAME).so.$(LIBVERSION) -o $(LIBRARY) $(SRCS)
	ln -sf $(LIBRARY) lib$(LIBNAME).so.$(LIBVERSION)
endif

install: $(LIBRARY)
	@echo "Installing files in $(DESTDIR) ..."
	install -d $(DESTDIR)
	install -m 0755 $< $(DESTDIR)

clean:
	rm -f *.o

.PHONY: all clean
