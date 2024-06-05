#====================================================================================
# RDK MANAGEMENT, LLC CONFIDENTIAL AND PROPRIETARY
#====================================================================================
# This file (and its contents) are the intellectual property of RDK Management, LLC.
# It may not be used, copied, distributed or otherwise  disclosed in whole or in
# part without the express written permission of RDK Management, LLC.
#====================================================================================
# Copyright (c) 2024 RDK Management, LLC. All rights reserved.
#====================================================================================
ACLOCAL_AMFLAGS = -Im4 --install

AM_CPPFLAGS = -g -Wall -std=c++0x -DGCC4_XXX \
	      -fPIC ${COPTS}

AM_CXXFLAGS = ${STORAGEMGR_FLAGS} -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
	      -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rdk/iarmbus \
	      -I${top_srcdir}/StorageMgr_stub/include -I${top_srcdir}/agent/include

lib_LTLIBRARIES = libstoragemanagerstub.la
libstoragemanagerstub_la_SOURCES = src/StorageMgrAgent.cpp
libstoragemanagerstub_la_includedir = ${includedir}
libstoragemanagerstub_la_LIBADD = -lrdkstmgr $(STORAGEMGR_LIBS)
libstoragemanagerstub_la_include_HEADERS = ./include/StorageMgrAgent.h
