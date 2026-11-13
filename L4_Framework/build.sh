#!/bin/bash

################################################################################
# MediaPipelineTests Build Script
# Purpose: Build MediaPipelineTests natively or with a generic/Yocto SDK
# Usage: ./build.sh [OPTIONS]
#
# Automatically detects sourced Yocto/RDK environments and builds ut-core if needed.
################################################################################

set -e  # Exit on any error

# Color output for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IS_YOCTO=0
if [ -n "$SDKTARGETSYSROOT" ] && [ -n "$OECORE_NATIVE_SYSROOT" ]; then
    IS_YOCTO=1
fi
BUILD_DIR="${PROJECT_DIR}/build"
INSTALL_PREFIX="${SDKTARGETSYSROOT}/usr"
BUILD_TYPE="Release"
THREADS=${THREADS:-$(nproc 2>/dev/null || echo 4)}
VERBOSE=0
CLEAN_BUILD=0
INSTALL=$IS_YOCTO
PLATFORM="${PLATFORM:-broadcom}"
TARGET_ARCH="unknown"

################################################################################
# Functions
################################################################################

print_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build MediaPipelineTests framework with cross-compilation support.
Automatically detects and builds ut-core if not present.
When both SDKTARGETSYSROOT and OECORE_NATIVE_SYSROOT are set, this script
uses the RDK/Yocto toolchain and installs by default. Otherwise it builds
natively or with a generic SDK.
All modes write to build/. Use --clean when switching between native and SDK builds.

OPTIONS:
    -h, --help              Show this help message
    -c, --clean             Clean build directory before building
    -i, --install           Install after successful build
    --no-install            Skip installation (Yocto installs by default)
    -p, --prefix PATH       Installation prefix (default: \$SDKTARGETSYSROOT/usr)
    -b, --build-type TYPE   Debug or Release (default: Release)
    -t, --threads N         Number of parallel build threads (default: nproc)
    -v, --verbose           Enable verbose output
    --platform PLATFORM     Target platform: broadcom, amlogic, realtek (default: broadcom)
    --toolchain FILE        CMake toolchain file path
    --ut-core-dir PATH      Path to ut-core installation (auto-detected if not set)
    --config-path PATH      Installation path for platform.ini

UT-CORE BEHAVIOR:
    The script automatically handles ut-core:
    1. Checks if UT_CORE_DIR environment variable is set
    2. Looks for ut-core in sibling directory (../ut-core)
    3. Looks for ut-core in project directory (./ut-core)
    4. Checks system installation via pkg-config
    5. If not found anywhere, clones from GitHub and builds it automatically
    
    To use custom ut-core location:
    export UT_CORE_DIR=/path/to/ut-core
    ./build.sh

EXAMPLES:
    # Standard SDK cross-compile build (auto-builds ut-core if needed)
    ./build.sh

    # RDK/Yocto build (source the SDK environment first)
    source /path/to/environment-setup
    ./build.sh

    # Clean rebuild with installation
    ./build.sh --clean --install

    # Debug build with verbose output
    ./build.sh --build-type Debug --verbose

    # Use existing ut-core
    export UT_CORE_DIR=/opt/ut-core
    ./build.sh

    # Custom prefix and toolchain
    ./build.sh --prefix /opt/mpt --toolchain /path/to/toolchain.cmake

ENVIRONMENT VARIABLES:
    PROJECT_DIR             Project root (auto-detected)
    SDKTARGETSYSROOT        SDK sysroot for cross-compile (from SDK)
    OECORE_NATIVE_SYSROOT   OE native sysroot (Yocto)
    CMAKE_TOOLCHAIN_FILE    Path to CMake toolchain
    UT_CORE_DIR             Path to ut-core (auto-detected/cloned if not set)
    THREADS                 Number of build threads

EOF
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

################################################################################
# Parse Arguments
################################################################################

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            print_usage
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -i|--install)
            INSTALL=1
            shift
            ;;
        --no-install)
            INSTALL=0
            shift
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -b|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -t|--threads)
            THREADS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        --platform)
            PLATFORM="$2"
            shift 2
            ;;
        --toolchain)
            CMAKE_TOOLCHAIN_FILE="$2"
            shift 2
            ;;
        --ut-core-dir)
            UT_CORE_DIR="$2"
            shift 2
            ;;
        --config-path)
            MP_CONFIG_INSTALL_PATH="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

################################################################################
# Validation
################################################################################

print_info "MediaPipelineTests Build Script"
print_info "================================"
echo ""

# Check if project directory exists
if [ ! -f "${PROJECT_DIR}/CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found in ${PROJECT_DIR}"
    print_error "Are you running this script from the project root?"
    exit 1
fi

# Check for required tools
for cmd in cmake make; do
    if ! command -v $cmd &> /dev/null; then
        print_error "$cmd not found. Please install it."
        exit 1
    fi
done

print_success "Project directory: ${PROJECT_DIR}"
print_success "Build type: ${BUILD_TYPE}"
print_success "Build threads: ${THREADS}"
print_success "Installation prefix: ${INSTALL_PREFIX}"
print_success "Target platform: ${PLATFORM}"

################################################################################
# SDK Environment Detection
################################################################################

print_info "SDK Environment Setup"
echo ""

if [ -n "$SDKTARGETSYSROOT" ]; then
    print_success "SDK sysroot detected: ${SDKTARGETSYSROOT}"
else
    print_warning "SDKTARGETSYSROOT not set (native build)"
fi

if [ -n "$OECORE_NATIVE_SYSROOT" ]; then
    print_success "OE environment detected"
    if [ -z "$CMAKE_TOOLCHAIN_FILE" ]; then
        # Search standard locations first; Yocto SDKs can also keep the
        # toolchain in a versioned CMake directory.
        CMAKE_SEARCH_PATHS=(
            "${OECORE_NATIVE_SYSROOT}/usr/share/cmake/OEToolchainConfig.cmake"
            "${OECORE_NATIVE_SYSROOT}/usr/share/cmake/OEToolchain.cmake"
            "${SDKTARGETSYSROOT}/../OEToolchain.cmake"
            "${OECORE_NATIVE_SYSROOT}"/usr/share/cmake-*/OEToolchainConfig.cmake
            "${OECORE_NATIVE_SYSROOT}"/usr/share/cmake-*/OEToolchain.cmake
        )
        
        for toolchain_path in "${CMAKE_SEARCH_PATHS[@]}"; do
            # Handle glob patterns by expanding them
            for expanded_path in $toolchain_path; do
                if [ -f "$expanded_path" ]; then
                    CMAKE_TOOLCHAIN_FILE="$expanded_path"
                    print_success "Using OE toolchain: ${CMAKE_TOOLCHAIN_FILE}"
                    break 2
                fi
            done
        done
        
        if [ -z "$CMAKE_TOOLCHAIN_FILE" ]; then
            if [ $IS_YOCTO -eq 1 ]; then
                CMAKE_TOOLCHAIN_FILE=$(find "$OECORE_NATIVE_SYSROOT" \
                    -name 'OEToolchain*.cmake' -print -quit 2>/dev/null || true)
            fi
            if [ -z "$CMAKE_TOOLCHAIN_FILE" ]; then
                print_warning "OE toolchain file not found"
                print_warning "Searched: ${CMAKE_SEARCH_PATHS[0]}"
            fi
        fi
    fi
fi

if [ $IS_YOCTO -eq 1 ]; then
    print_success "RDK/Yocto environment detected"
    print_success "OE native sysroot: ${OECORE_NATIVE_SYSROOT}"
    print_success "SDK machine: ${SDKMACHINE:-not set}"
    print_success "Target machine: ${MACHINE:-not set}"
    if [ -d "$SDKTARGETSYSROOT/lib64" ] && [ ! -L "$SDKTARGETSYSROOT/lib64" ]; then
        TARGET_ARCH="x86_64"
    elif [ -d "$SDKTARGETSYSROOT/lib/aarch64-linux-gnu" ]; then
        TARGET_ARCH="aarch64"
    elif [ -d "$SDKTARGETSYSROOT/lib/arm-linux-gnueabihf" ]; then
        TARGET_ARCH="armv7l"
    elif [ -d "$SDKTARGETSYSROOT/lib/arm-linux-gnueabi" ]; then
        TARGET_ARCH="armv6l"
    else
        TARGET_ARCH="arm"
    fi
    print_success "Target architecture: ${TARGET_ARCH}"
    if [ -z "$CMAKE_TOOLCHAIN_FILE" ] || [ ! -f "$CMAKE_TOOLCHAIN_FILE" ]; then
        print_error "CMake OEToolchain file not found; source the RDK SDK environment correctly"
        exit 1
    fi
fi

# Set default config path if not specified
if [ -z "$MP_CONFIG_INSTALL_PATH" ]; then
    MP_CONFIG_INSTALL_PATH="/etc/rdk/mediapipeline-tests/platform.ini"
fi

print_success "Config install path: ${MP_CONFIG_INSTALL_PATH}"

# Setup pkg-config paths for cross-compilation in OE SDK
if [ -n "$SDKTARGETSYSROOT" ]; then
    print_info "Configuring pkg-config for OE cross-compilation..."
    
    # Use OE SDK's pkg-config
    export PKG_CONFIG_SYSROOT_DIR="${SDKTARGETSYSROOT}"
    export PKG_CONFIG_LIBDIR="${SDKTARGETSYSROOT}/usr/lib/pkgconfig:${SDKTARGETSYSROOT}/usr/share/pkgconfig"
    # Don't inherit host pkg-config paths
    unset PKG_CONFIG_PATH
    
    print_success "PKG_CONFIG configured for OE SDK sysroot"
    print_info "  PKG_CONFIG_SYSROOT_DIR: ${PKG_CONFIG_SYSROOT_DIR}"
    print_info "  PKG_CONFIG_LIBDIR: ${PKG_CONFIG_LIBDIR}"
    
    # Verify libpcre can be found
    if pkg-config --exists libpcre 2>/dev/null; then
        print_success "libpcre found via pkg-config"
    else
        print_warning "libpcre not found in pkg-config"
        print_warning "Check the sourced SDK's pkg-config paths and sysroot packages"
    fi
fi
echo ""

################################################################################
# ut-core Detection and Build
################################################################################

print_info "Checking ut-core Framework"
echo ""

# Function to build ut-core
build_ut_core() {
    local ut_core_path=$1
    print_info "Building ut-core framework..."
    
    # Save current directory
    local current_dir=$(pwd)
    
    cd "$ut_core_path"
    
    # Fix Perl paths for OE SDK environment
    if [ -n "$OECORE_NATIVE_SYSROOT" ]; then
        print_info "OE SDK detected, setting up Perl environment..."
        export PERL5LIB="${OECORE_NATIVE_SYSROOT}/usr/lib/perl5/5.34.3:${OECORE_NATIVE_SYSROOT}/usr/lib/perl5/5.34.3/x86_64-linux"
        export PATH="${OECORE_NATIVE_SYSROOT}/usr/bin:${PATH}"
    fi
    
    # Build ut-core with target platform if needed
    if [ -x "./build.sh" ]; then
        if [ $IS_YOCTO -eq 1 ]; then
            if ! TARGET=arm ./build.sh; then
                print_warning "ut-core build.sh failed, trying native Makefile build..."
                if [ -f "Makefile" ]; then
                    if ! make TARGET=arm; then
                        print_error "ut-core build failed"
                        cd "$current_dir"
                        exit 1
                    fi
                else
                    print_error "ut-core build script or Makefile not found"
                    cd "$current_dir"
                    exit 1
                fi
            fi
        elif ! ./build.sh; then
            print_warning "ut-core build.sh failed, trying native Makefile build..."
            if [ -f "Makefile" ]; then
                if ! make; then
                    print_error "ut-core build failed (both build.sh and make)"
                    print_error "This may be due to Perl module issues in OE SDK"
                    print_error "Workarounds:"
                    print_error "  1. Build ut-core outside SDK environment: git clone https://github.com/rdkcentral/ut-core.git && cd ut-core && ./build.sh"
                    print_error "  2. Install ut-core in OE SDK: bitbake ut-core"
                    print_error "  3. Specify system ut-core: export UT_CORE_DIR=/path/to/ut-core"
                    cd "$current_dir"
                    exit 1
                fi
            else
                print_error "ut-core build script or Makefile not found"
                cd "$current_dir"
                exit 1
            fi
        fi
    elif [ -f "Makefile" ]; then
        if [ $IS_YOCTO -eq 1 ]; then
            if ! make TARGET=arm; then
                print_error "ut-core make failed"
                cd "$current_dir"
                exit 1
            fi
        elif ! make; then
            print_error "ut-core make failed"
            cd "$current_dir"
            exit 1
        fi
    else
        print_error "ut-core build script or Makefile not found"
        cd "$current_dir"
        exit 1
    fi
    
    cd "$current_dir"
    print_success "ut-core built successfully"
}

# Check if ut-core is provided via environment variable
if [ -n "$UT_CORE_DIR" ] && [ -d "$UT_CORE_DIR" ]; then
    print_success "ut-core provided: ${UT_CORE_DIR}"
else
    # Look for ut-core in common locations
    UT_CORE_DIR=""

    # Prefer the SDK's installed ut-core when building with Yocto.
    if [ $IS_YOCTO -eq 1 ] && pkg-config --exists "ut-core" 2>/dev/null; then
        UT_CORE_DIR=$(pkg-config --variable=libdir ut-core)
        print_success "ut-core found in Yocto environment: ${UT_CORE_DIR}"
    # Check 1: Sibling directory
    elif [ -d "${PROJECT_DIR}/../ut-core" ]; then
        UT_CORE_DIR="${PROJECT_DIR}/../ut-core"
        print_success "ut-core found (sibling): ${UT_CORE_DIR}"
    # Check 2: Current workspace
    elif [ -d "${PROJECT_DIR}/ut-core" ]; then
        UT_CORE_DIR="${PROJECT_DIR}/ut-core"
        print_success "ut-core found (local): ${UT_CORE_DIR}"
    # Check 3: System installation
    elif pkg-config --exists "ut-core" 2>/dev/null; then
        UT_CORE_DIR=$(pkg-config --variable=libdir ut-core)
        print_success "ut-core found (system): ${UT_CORE_DIR}"
    else
        # ut-core not found, clone and build it
        print_warning "ut-core not found, cloning from repository..."
        
        UT_REPO="https://github.com/rdkcentral/ut-core.git"
        UT_CORE_DIR="${PROJECT_DIR}/ut-core"
        
        # Clone ut-core
        if ! git clone "${UT_REPO}" "${UT_CORE_DIR}"; then
            print_error "Failed to clone ut-core from ${UT_REPO}"
            exit 1
        fi
        print_success "ut-core cloned to: ${UT_CORE_DIR}"
        
        # Build ut-core
        build_ut_core "${UT_CORE_DIR}"
    fi
fi

# Verify ut-core exists and is functional
if [ -z "$UT_CORE_DIR" ] || [ ! -d "$UT_CORE_DIR" ]; then
    print_error "ut-core directory not found or invalid"
    exit 1
fi

# Check if ut-core needs to be built
if [ ! -f "${UT_CORE_DIR}/build/.install_manifest.txt" ] && \
   [ ! -f "${UT_CORE_DIR}/lib/libut.a" ] && \
   [ ! -f "${UT_CORE_DIR}/lib/libut.so" ] && \
   [ ! -f "${UT_CORE_DIR}/lib/libut_control.a" ] && \
   [ ! -f "${UT_CORE_DIR}/lib/libut_control.so" ] && \
   [ ! -f "${UT_CORE_DIR}/build/lib/libut.so" ] && \
   [ ! -f "${UT_CORE_DIR}/build/lib/libut_control.so" ]; then
    print_warning "ut-core not built yet, building now..."
    build_ut_core "${UT_CORE_DIR}"
fi

print_success "ut-core ready: ${UT_CORE_DIR}"
echo ""

if [ $IS_YOCTO -eq 1 ]; then
    print_info "RDK/Yocto mode: Release build, installation enabled by default"
fi

################################################################################
# Clean Build Directory
################################################################################

if [ $CLEAN_BUILD -eq 1 ]; then
    print_info "Cleaning previous build..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    fi
fi

################################################################################
# Create Build Directory
################################################################################

print_info "Preparing build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
print_success "Build directory: ${BUILD_DIR}"
echo ""

################################################################################
# CMake Configuration
################################################################################

print_info "Configuring CMake..."
echo ""

CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    "-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}"
    "-DMP_CONFIG_INSTALL_PATH=${MP_CONFIG_INSTALL_PATH}"
    "-DPLATFORM=${PLATFORM}"
    "-DUT_CORE_DIR=${UT_CORE_DIR}"
)

# Add cross-compilation flags for OE SDK
if [ -n "$SDKTARGETSYSROOT" ]; then
    CMAKE_ARGS+=(
        "-DCMAKE_FIND_ROOT_PATH=${SDKTARGETSYSROOT}"
        "-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER"
        "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY"
        "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY"
        "-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY"
    )
fi

# Add toolchain if specified or detected
if [ -n "$CMAKE_TOOLCHAIN_FILE" ]; then
    if [ ! -f "$CMAKE_TOOLCHAIN_FILE" ]; then
        print_warning "Toolchain file not found: ${CMAKE_TOOLCHAIN_FILE}"
        print_warning "Attempting native build (may fail for cross-compilation)"
    else
        CMAKE_ARGS+=("-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
        print_success "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}"
    fi
fi

# Add verbose flag
if [ $VERBOSE -eq 1 ]; then
    CMAKE_ARGS+=("--debug-output")
    print_success "Verbose mode enabled"
fi

CMAKE_ARGS+=("${PROJECT_DIR}")

print_info "CMake arguments:"
for arg in "${CMAKE_ARGS[@]}"; do
    echo "    $arg"
done
echo ""

if ! cmake "${CMAKE_ARGS[@]}"; then
    print_error "CMake configuration failed"
    
    # Check if this might be a Yocto/RDK environment issue
    if [ -n "$OECORE_NATIVE_SYSROOT" ] || [ -n "$SDKTARGETSYSROOT" ]; then
        echo ""
        print_info "You appear to be using a Yocto/RDK SDK environment."
        print_info "For RDK builds, source the complete Yocto SDK environment before running build.sh."
    fi
    exit 1
fi

print_success "CMake configuration completed"
echo ""

################################################################################
# Compilation
################################################################################

print_info "Building..."
echo ""

BUILD_CMD="make -j${THREADS}"

if [ $VERBOSE -eq 1 ]; then
    BUILD_CMD="${BUILD_CMD} VERBOSE=1"
fi

print_info "Build command: ${BUILD_CMD}"
echo ""

if ! ${BUILD_CMD}; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully"
echo ""

################################################################################
# Build Verification
################################################################################

print_info "Verifying build artifacts..."
echo ""

if [ -f "tdk_mediapipelinetest" ]; then
    SIZE=$(ls -lh tdk_mediapipelinetest | awk '{print $5}')
    print_success "Executable created: tdk_mediapipelinetest (${SIZE})"
    
    # Show architecture
    if command -v file &> /dev/null; then
        ARCH=$(file tdk_mediapipelinetest | grep -o "ARM\|x86\|aarch64" || echo "unknown")
        print_info "Architecture: ${ARCH}"
    fi
else
    print_error "Executable not found: tdk_mediapipelinetest"
    exit 1
fi

# Check linked libraries
if command -v ldd &> /dev/null; then
    print_info "Linked libraries:"
    ldd tdk_mediapipelinetest 2>/dev/null | grep -E "gstreamer|curl|glib" | sed 's/^/    /'
fi

echo ""

################################################################################
# Installation
################################################################################

if [ $INSTALL -eq 1 ]; then
    print_info "Installing..."
    echo ""
    
    if ! make install; then
        print_error "Installation failed"
        exit 1
    fi
    
    print_success "Installation completed"
    print_info "Installed to: ${INSTALL_PREFIX}"
    echo ""
fi

################################################################################
# Summary
################################################################################

print_success "Build Summary"
echo "=============================================="
echo "  Project: ${PROJECT_DIR}"
echo "  Build Directory: ${BUILD_DIR}"
echo "  Build Type: ${BUILD_TYPE}"
echo "  Platform: ${PLATFORM}"
if [ $IS_YOCTO -eq 1 ]; then
    echo "  Target Architecture: ${TARGET_ARCH}"
fi
echo "  Executable: ${BUILD_DIR}/tdk_mediapipelinetest"
if [ $INSTALL -eq 1 ]; then
    echo "  Installed To: ${INSTALL_PREFIX}"
fi
echo "=============================================="
echo ""

print_success "Build completed successfully! ✓"
echo ""

# Print next steps
echo "Next steps:"
echo "  1. Run tests:"
echo "     ${BUILD_DIR}/tdk_mediapipelinetest"
echo ""
echo "  2. Run specific test:"
echo "     ${BUILD_DIR}/tdk_mediapipelinetest --test-name test_play_pause_pipeline"
echo ""
echo "  3. Enable verbose logging:"
echo "     GST_DEBUG=3 ${BUILD_DIR}/tdk_mediapipelinetest"
echo ""

exit 0
