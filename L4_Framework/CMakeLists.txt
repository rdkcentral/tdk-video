#[[ File: CMakeLists.txt
    Purpose: Build configuration for MediaPipelineTests framework with ut-core integration
]]
cmake_minimum_required(VERSION 3.19)
project(MediaPipelineTests VERSION 1.0 LANGUAGES C CXX)

include(GNUInstallDirs)

set(MP_CONFIG_INSTALL_PATH "/etc/rdk/mediapipeline-tests/platform.ini" CACHE FILEPATH
    "Installed path of the default test configuration")

# Find dependencies
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# For cross-compilation with OE/Yocto SDK: explicitly set search paths
# SDK sysroot is passed via CMAKE_FIND_ROOT_PATH from build scripts
set(SDK_SYSROOT "${CMAKE_FIND_ROOT_PATH}" CACHE PATH "SDK sysroot path")

# GStreamer/GLib dependencies - use find_library with explicit SDK paths
# This avoids pkg-config's transitive dependency checking for missing .pc files (libpcre)
find_library(GST_LIBRARY 
    NAMES gstreamer-1.0
    HINTS "${SDK_SYSROOT}/usr/lib" "${SDK_SYSROOT}/usr/lib64"
    REQUIRED)

find_path(GST_INCLUDE_DIR 
    NAMES gst/gst.h
    HINTS "${SDK_SYSROOT}/usr/include" "${SDK_SYSROOT}/usr/include/gstreamer-1.0"
    REQUIRED)

find_library(GLIB_LIBRARY 
    NAMES glib-2.0
    HINTS "${SDK_SYSROOT}/usr/lib" "${SDK_SYSROOT}/usr/lib64"
    REQUIRED)

find_library(GOBJECT_LIBRARY
    NAMES gobject-2.0
    HINTS "${SDK_SYSROOT}/usr/lib" "${SDK_SYSROOT}/usr/lib64"
    REQUIRED)

find_path(GLIB_INCLUDE_DIR 
    NAMES glib.h
    HINTS "${SDK_SYSROOT}/usr/include/glib-2.0" "${SDK_SYSROOT}/usr/include"
    REQUIRED)

# Also find glibconfig.h which is typically in the lib directory
find_path(GLIBCONFIG_INCLUDE_DIR 
    NAMES glibconfig.h
    HINTS "${SDK_SYSROOT}/usr/lib/glib-2.0/include" "${SDK_SYSROOT}/usr/lib64/glib-2.0/include")

# Create GST variables for compatibility with rest of CMakeLists.txt
set(GST_LIBRARIES ${GST_LIBRARY} ${GOBJECT_LIBRARY} ${GLIB_LIBRARY})
set(GST_INCLUDE_DIRS ${GST_INCLUDE_DIR} ${GLIB_INCLUDE_DIR})

# Add glibconfig.h directory if found
if(GLIBCONFIG_INCLUDE_DIR)
    list(APPEND GST_INCLUDE_DIRS ${GLIBCONFIG_INCLUDE_DIR})
endif()

# Add gstreamer plugin directory if available
find_path(GST_PLUGINS_DIR NAMES libgstcoreelements.so
    HINTS "${SDK_SYSROOT}/usr/lib/gstreamer-1.0" "${SDK_SYSROOT}/usr/lib64/gstreamer-1.0")

# UT Core dependencies - find pre-built ut-core library
# UT_CORE_DIR is passed via -DUT_CORE_DIR from build scripts
if(DEFINED UT_CORE_DIR)
    # ut-core is expected to be pre-built in UT_CORE_DIR
    # The library is named libut_control.so from ut-core's build
    find_library(UT_CORE_LIBRARY
        NAMES ut_control ut ut_core utcore ut_cunit
        HINTS "${UT_CORE_DIR}/lib" "${UT_CORE_DIR}/build/lib" "${UT_CORE_DIR}/build"
        NO_DEFAULT_PATH)
    
    # If library not found, provide helpful diagnostic and search more broadly
    if(NOT UT_CORE_LIBRARY)
        message(STATUS "ut-core library not found in standard locations, searching...")
        # Search recursively for any .a or .so files named *ut*
        file(GLOB_RECURSE UT_CORE_CANDIDATES "${UT_CORE_DIR}/*ut*.a" "${UT_CORE_DIR}/*ut*.so")
        if(UT_CORE_CANDIDATES)
            list(GET UT_CORE_CANDIDATES 0 UT_CORE_LIBRARY)
            message(STATUS "Found ut-core library: ${UT_CORE_LIBRARY}")
        else()
            message(FATAL_ERROR "Could not find ut-core library in ${UT_CORE_DIR}. 
                The library should be named libut_control.so or similar.
                Please check that ut-core was built successfully by running:
                ls -la ${UT_CORE_DIR}/lib/
                ls -la ${UT_CORE_DIR}/build/lib/
                find ${UT_CORE_DIR} -name '*ut*.a' -o -name '*ut*.so'")
        endif()
    else()
        message(STATUS "Found ut-core library: ${UT_CORE_LIBRARY}")
    endif()
   
    # Add diagnostic message
    message(STATUS "UT_CORE_LIBRARY = ${UT_CORE_LIBRARY}")

    find_path(UT_CORE_INCLUDE_DIR ut.h
        HINTS "${UT_CORE_DIR}/include"
              "${UT_CORE_DIR}"
              "${SDK_SYSROOT}/usr/include"
        PATH_SUFFIXES "ut-core"
        NO_DEFAULT_PATH)
    
    if(NOT UT_CORE_INCLUDE_DIR)
        # Fallback: search recursively for ut.h
        file(GLOB_RECURSE UT_CORE_INCLUDE_SEARCH "${UT_CORE_DIR}/*/ut.h")
        if(UT_CORE_INCLUDE_SEARCH)
            get_filename_component(UT_CORE_INCLUDE_DIR "${UT_CORE_INCLUDE_SEARCH}" DIRECTORY)
            message(STATUS "Found ut-core includes (via recursive search): ${UT_CORE_INCLUDE_DIR}")
        else()
            message(FATAL_ERROR "Could not find ut.h in ut-core directory.
                Please check that ut-core was built successfully by running:
                find ${UT_CORE_DIR} -name 'ut.h'")
        endif()
    else()
        message(STATUS "Found ut-core includes: ${UT_CORE_INCLUDE_DIR}")
    endif()

    find_path(UT_LOG_INCLUDE_DIR ut_log.h
        HINTS "${UT_CORE_DIR}/include"
              "${UT_CORE_DIR}/framework/ut-control/include/"
              "${SDK_SYSROOT}/usr/include"
        PATH_SUFFIXES "ut-core"
        NO_DEFAULT_PATH)

    if(NOT UT_LOG_INCLUDE_DIR)
        # Fallback: search recursively for ut.h
	file(GLOB_RECURSE UT_LOG_INCLUDE_SEARCH "${UT_CORE_DIR}/*/ut_log.h")
	if(UT_LOG_INCLUDE_SEARCH)
	    get_filename_component(UT_LOG_INCLUDE_DIR "${UT_LOG_INCLUDE_SEARCH}" DIRECTORY)
	    message(STATUS "Found ut_log includes (via recursive search): ${UT_LOG_INCLUDE_DIR}")
        else()
            message(FATAL_ERROR "Could not find ut_log.h in ut-core directory.
                Please check that ut-core was built successfully by running:
                find ${UT_CORE_DIR} -name 'ut_log.h'")
        endif()
    else()
	message(STATUS "Found ut_log includes: ${UT_LOG_INCLUDE_DIR}")
    endif()
else()
    message(FATAL_ERROR "UT_CORE_DIR not specified. Pass -DUT_CORE_DIR=/path/to/ut-core")
endif()

# UT Core and other dependencies
pkg_check_modules(GTEST REQUIRED gtest)
#pkg_check_modules(CURL REQUIRED libcurl)

find_path(GTEST_INCLUDE_DIR gtest/gtest.h REQUIRED)
find_library(GTEST_LIBRARY gtest REQUIRED)

# Try to find CUnit headers (bundled with ut-core)
# CUnit is located in: ${UT_CORE_DIR}/framework/CUnit-2.1-3/CUnit/Headers/CUnit.h
# Try both possible include path structures
find_path(CUNIT_INCLUDE_DIR
    NAMES CUnit.h
    HINTS "${UT_CORE_DIR}/framework/CUnit-2.1-3/CUnit/Headers"
    NO_DEFAULT_PATH)

if(NOT CUNIT_INCLUDE_DIR)
    # Try alternate path structure (in case it's CUnit/CUnit.h)
    find_path(CUNIT_INCLUDE_DIR
        NAMES CUnit/CUnit.h
        HINTS "${UT_CORE_DIR}/framework/CUnit-2.1-3"
        NO_DEFAULT_PATH)
endif()

if(NOT CUNIT_INCLUDE_DIR)
    # Search recursively in ut-core directory for CUnit.h
    file(GLOB_RECURSE CUNIT_HEADER_SEARCH "${UT_CORE_DIR}/*/CUnit.h")
    if(CUNIT_HEADER_SEARCH)
        # Get the directory containing CUnit.h
        get_filename_component(CUNIT_INCLUDE_DIR "${CUNIT_HEADER_SEARCH}" DIRECTORY)
    endif()
endif()

if(CUNIT_INCLUDE_DIR)
    message(STATUS "Found CUnit headers at: ${CUNIT_INCLUDE_DIR}")
    set(HAVE_CUNIT TRUE)
else()
    message(WARNING "CUnit headers not found - test registration API (UT_add_suite, UT_add_test) will be unavailable.
        Expected location: ${UT_CORE_DIR}/framework/CUnit-2.1-3/CUnit/Headers/")
    set(HAVE_CUNIT FALSE)
endif()

if(HAVE_CUNIT)
    file(GLOB_RECURSE UT_CORE_CUNIT_SOURCES CONFIGURE_DEPENDS
        "${UT_CORE_DIR}/src/*.c")
    file(GLOB CUNIT_FRAMEWORK_SOURCES CONFIGURE_DEPENDS
        "${UT_CORE_DIR}/framework/CUnit-2.1-3/CUnit/Sources/Framework/*.c")

    add_library(ut_core_cunit STATIC
        ${UT_CORE_CUNIT_SOURCES}
        ${CUNIT_FRAMEWORK_SOURCES}
    )
    target_compile_definitions(ut_core_cunit PRIVATE
        UT_CUNIT
        UT_VERSION="MediaPipelineTests"
    )
    target_include_directories(ut_core_cunit PRIVATE
        ${UT_CORE_INCLUDE_DIR}
        ${UT_LOG_INCLUDE_DIR}
        ${CUNIT_INCLUDE_DIR}
        ${UT_CORE_DIR}/src
        ${UT_CORE_DIR}/src/c_source
    )
    target_link_libraries(ut_core_cunit PUBLIC
        ${UT_CORE_LIBRARY}
        Threads::Threads
    )
endif()

# MediaPipeline framework library
file(GLOB FYAML_INCLUDE_CANDIDATES
    "${UT_CORE_DIR}/framework/ut-control/framework/*/libfyaml-master/include")
find_path(FYAML_INCLUDE_DIR libfyaml.h
    HINTS ${FYAML_INCLUDE_CANDIDATES}
    NO_CMAKE_FIND_ROOT_PATH
    REQUIRED)

add_library(mediapipelinetests_framework
    src/mediapipelinetests_config_impl.cpp
    src/mediapipelinetests_environment.cpp
    src/mediapipelinetests_pipeline_impl.cpp
    src/mediapipelinetests_verification_impl.cpp
    src/mediapipelinetests_framework_impl.cpp
)

target_compile_features(mediapipelinetests_framework PUBLIC cxx_std_17)
target_compile_definitions(mediapipelinetests_framework PRIVATE MP_DEFAULT_CONFIG_PATH="${MP_CONFIG_INSTALL_PATH}")
# Enable UT_CUNIT for mediapipelinetests_framework if CUnit headers are available
if(HAVE_CUNIT)
    target_compile_definitions(mediapipelinetests_framework PRIVATE UT_CUNIT)
    target_include_directories(mediapipelinetests_framework PRIVATE ${CUNIT_INCLUDE_DIR})
endif()
target_include_directories(mediapipelinetests_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GST_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${UT_CORE_INCLUDE_DIR}
    ${UT_LOG_INCLUDE_DIR}
)
target_include_directories(mediapipelinetests_framework PRIVATE ${FYAML_INCLUDE_DIR})

target_link_libraries(mediapipelinetests_framework PUBLIC
    ${GST_LIBRARIES}
    ${CURL_LIBRARIES}
    ut_core_cunit
    ${CMAKE_DL_LIBS}
    Threads::Threads
)

# Main executable
add_executable(tdk_mediapipelinetest
    src/mediapipelinetests_main.cpp
    src/mediapipelinetests_test_suite.cpp
)

target_compile_features(tdk_mediapipelinetest PRIVATE cxx_std_17)
target_compile_definitions(tdk_mediapipelinetest PRIVATE MP_DEFAULT_CONFIG_PATH="${MP_CONFIG_INSTALL_PATH}")

# Only enable UT_CUNIT if CUnit headers are available
if(HAVE_CUNIT)
    target_compile_definitions(tdk_mediapipelinetest PRIVATE UT_CUNIT)
    target_include_directories(tdk_mediapipelinetest PRIVATE ${CUNIT_INCLUDE_DIR})
endif()

target_include_directories(tdk_mediapipelinetest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${UT_CORE_INCLUDE_DIR}
    ${UT_LOG_INCLUDE_DIR}
)

target_link_libraries(tdk_mediapipelinetest PRIVATE
    mediapipelinetests_framework
    ${GST_LIBRARIES}
    ${CURL_LIBRARIES}
    Threads::Threads
)

# Install configuration
install(FILES config/platform.ini config/env.yaml
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/rdk/mediapipeline-tests
)

# TODO: Install video test assets
# install(DIRECTORY video-inputs/
#     DESTINATION ${CMAKE_INSTALL_DATADIR}/rdk/mediapipeline-tests/video-inputs
# )
