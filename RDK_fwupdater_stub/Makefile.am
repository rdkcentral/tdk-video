##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2025 RDK Management
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
ACLOCAL_AMFLAGS = -Im4 --install

AM_CPPFLAGS = -g -Wall -std=c++0x -DGCC4_XXX \
              -fPIC ${COPTS}

AM_CXXFLAGS = ${RDK_fwupdater_FLAGS} -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
	      -I${top_srcdir}/RDK_fwupdater_stub/src/rdkfwupdater_source/src/deviceutils \
	      -DRDK_LOGGER -DIARM_ENABLED -DRFC_API_ENABLED \
              -I${top_srcdir}/RDK_fwupdater_stub/include -I${top_srcdir}/agent/include

lib_LTLIBRARIES = librdkfwupgraderstub.la
librdkfwupgraderstub_la_SOURCES = src/RDK_fwupgraderAgent.cpp src/rdkfwupdater_source/src/deviceutils/deviceutils.c
librdkfwupgraderstub_la_includedir = ${includedir}
librdkfwupgraderstub_la_LIBADD = -lsecure_wrapper -lfwutils -lparsejson -ldwnlutil -lrdkloggers ${RDK_fwupdater_LIBS}
librdkfwupgraderstub_la_include_HEADERS = ${top_srcdir}/RDK_fwupdater_stub/include/RDK_fwupgradeAgent.h
