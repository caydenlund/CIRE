#!/bin/bash

# Static build script for CIRE
# Downloads and builds all dependencies (LLVM, IBEX) and builds CIRE with static linking
# Based on dependencies listed in README.md

set -e  # Exit on error
set -o pipefail  # Exit on pipe failure

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default configuration
LLVM_VERSION="${LLVM_VERSION:-21.1.7}"
IBEX_VERSION="${IBEX_VERSION:-2.8.9}"
INSTALL_PREFIX="${INSTALL_PREFIX:-${PWD}/build-deps}"
BUILD_DIR="${BUILD_DIR:-${PWD}/build-static-native}"
NUM_JOBS="${NUM_JOBS:-$(nproc 2>/dev/null || echo 4)}"
SKIP_LLVM="${SKIP_LLVM:-false}"
SKIP_IBEX="${SKIP_IBEX:-false}"
SKIP_CIRE="${SKIP_CIRE:-false}"
CLEAN="${CLEAN:-false}"

# Parse command line arguments
show_help() {
    cat << EOF
Usage: $0 [OPTIONS]

Build CIRE with all dependencies statically linked.

Options:
  --llvm-version VER     LLVM version to build (default: 21.1.7)
  --ibex-version VER     IBEX version to build (default: 2.8.9)
  --prefix DIR           Installation prefix (default: ./build-deps)
  --build-dir DIR        Build directory (default: ./build-static-native)
  --jobs N               Number of parallel build jobs (default: auto-detected)
  --skip-llvm            Skip LLVM build (use existing installation)
  --skip-ibex            Skip IBEX build (use existing installation)
  --skip-cire            Skip CIRE build (only build dependencies)
  --clean                Clean build directories before building
  --help, -h             Show this help message

Environment variables:
  LLVM_VERSION          Override LLVM version
  IBEX_VERSION          Override IBEX version
  INSTALL_PREFIX        Override installation prefix
  BUILD_DIR             Override build directory
  NUM_JOBS              Override number of parallel jobs

Examples:
  # Build everything with defaults
  $0

  # Build with specific LLVM version
  $0 --llvm-version 16.0.0

  # Skip LLVM build and use system installation
  $0 --skip-llvm

  # Clean build
  $0 --clean
EOF
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --llvm-version)
            LLVM_VERSION="$2"
            shift 2
            ;;
        --ibex-version)
            IBEX_VERSION="$2"
            shift 2
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --jobs)
            NUM_JOBS="$2"
            shift 2
            ;;
        --skip-llvm)
            SKIP_LLVM=true
            shift
            ;;
        --skip-ibex)
            SKIP_IBEX=true
            shift
            ;;
        --skip-cire)
            SKIP_CIRE=true
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Print banner
echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║          CIRE Static Build Script (Native)                ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Print configuration
echo -e "${GREEN}Configuration:${NC}"
echo "  LLVM Version: $LLVM_VERSION"
echo "  IBEX Version: $IBEX_VERSION"
echo "  Install Prefix: $INSTALL_PREFIX"
echo "  Build Directory: $BUILD_DIR"
echo "  Parallel Jobs: $NUM_JOBS"
echo "  Skip LLVM: $SKIP_LLVM"
echo "  Skip IBEX: $SKIP_IBEX"
echo "  Skip CIRE: $SKIP_CIRE"
echo "  Clean Build: $CLEAN"
echo ""

# Check required tools
echo -e "${BLUE}Checking dependencies...${NC}"

check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        echo "Please install $1 and try again"
        exit 1
    fi
}

check_command gcc
check_command g++
check_command cmake
check_command ninja
check_command bison
check_command flex
check_command wget
check_command tar

# Check GCC version
GCC_VERSION=$(gcc -dumpversion | cut -d. -f1)
if [ "$GCC_VERSION" -lt 11 ]; then
    echo -e "${RED}Error: GCC version 11 or higher required (found $GCC_VERSION)${NC}"
    echo "IBEX requires GCC 11+, CIRE requires GCC 13+"
    exit 1
fi

# Check for Python 2 (needed for IBEX waf)
if ! command -v python2 &> /dev/null && ! command -v python2.7 &> /dev/null; then
    echo -e "${RED}Error: Python 2.x is required for IBEX build${NC}"
    echo "Please install python2 and try again"
    exit 1
fi

echo -e "${GREEN}All required tools found${NC}"
echo ""

# Create directories
mkdir -p "$INSTALL_PREFIX"
mkdir -p "$BUILD_DIR"

# Convert to absolute paths
INSTALL_PREFIX=$(cd "$INSTALL_PREFIX" && pwd)
BUILD_DIR=$(cd "$BUILD_DIR" && pwd)

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directories...${NC}"
    rm -rf "$BUILD_DIR"/*
    rm -rf "$INSTALL_PREFIX"/*
    mkdir -p "$BUILD_DIR"
    mkdir -p "$INSTALL_PREFIX"
fi

#===============================================================================
# Build LLVM
#===============================================================================

if [ "$SKIP_LLVM" = false ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  Building LLVM ${LLVM_VERSION}                                     ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""

    LLVM_INSTALL_DIR="$INSTALL_PREFIX/llvm"
    LLVM_BUILD_DIR="$BUILD_DIR/llvm-build"
    LLVM_SRC_DIR="$BUILD_DIR/llvm-project"

    if [ ! -d "$LLVM_INSTALL_DIR/bin" ]; then
        echo "Downloading LLVM $LLVM_VERSION..."
        cd "$BUILD_DIR"

        if [ ! -f "llvmorg-${LLVM_VERSION}.tar.gz" ]; then
            wget "https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-${LLVM_VERSION}.tar.gz"
        fi

        if [ ! -d "$LLVM_SRC_DIR" ]; then
            echo "Extracting LLVM source..."
            tar xzf "llvmorg-${LLVM_VERSION}.tar.gz"
            mv "llvm-project-llvmorg-${LLVM_VERSION}" llvm-project
        fi

        echo "Configuring LLVM..."
        mkdir -p "$LLVM_BUILD_DIR"
        cd "$LLVM_BUILD_DIR"

        cmake "$LLVM_SRC_DIR/llvm" \
            -GNinja \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX="$LLVM_INSTALL_DIR" \
            -DLLVM_ENABLE_DUMP=ON \
            -DLLVM_ENABLE_ASSERTIONS=ON \
            -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lld" \
            -DLLVM_TARGETS_TO_BUILD="X86" \
            -DLLVM_PARALLEL_LINK_JOBS=1 \
            -DLLVM_OPTIMIZED_TABLEGEN=ON \
            -DLLVM_ENABLE_EH=ON \
            -DLLVM_ENABLE_RTTI=ON \
            -DLLVM_BUILD_LLVM_DYLIB=OFF \
            -DBUILD_SHARED_LIBS=OFF \
            -DLLVM_INCLUDE_TESTS=OFF \
            -DLLVM_INCLUDE_EXAMPLES=OFF \
            -DLLVM_INCLUDE_BENCHMARKS=OFF

        echo ""
        echo -e "${YELLOW}Building LLVM (this will take 30-60 minutes)...${NC}"
        ninja -j"$NUM_JOBS"

        echo "Installing LLVM..."
        ninja install

        echo -e "${GREEN}LLVM build complete!${NC}"
        echo ""
    else
        echo -e "${YELLOW}LLVM already built, skipping...${NC}"
        echo ""
    fi
else
    echo -e "${YELLOW}Skipping LLVM build as requested${NC}"
    LLVM_INSTALL_DIR="${LLVM_INSTALL_DIR:-/usr/local}"
    echo "Using LLVM from: $LLVM_INSTALL_DIR"
    echo ""
fi

#===============================================================================
# Build IBEX
#===============================================================================

if [ "$SKIP_IBEX" = false ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  Building IBEX ${IBEX_VERSION}                                     ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""

    IBEX_INSTALL_DIR="$INSTALL_PREFIX/ibex"
    IBEX_SRC_DIR="$BUILD_DIR/ibex-src"

    if [ ! -d "$IBEX_INSTALL_DIR/lib" ]; then
        echo "Downloading IBEX $IBEX_VERSION..."
        cd "$BUILD_DIR"

        if [ ! -f "ibex-${IBEX_VERSION}.tar.gz" ]; then
            wget "https://github.com/ibex-team/ibex-lib/archive/refs/tags/ibex-${IBEX_VERSION}.tar.gz"
        fi

        if [ ! -d "$IBEX_SRC_DIR" ]; then
            echo "Extracting IBEX source..."
            tar xzf "ibex-${IBEX_VERSION}.tar.gz"
            mv "ibex-lib-ibex-${IBEX_VERSION}" ibex-src
        fi

        cd "$IBEX_SRC_DIR"

        # Ensure python2 is available
        if ! command -v python &> /dev/null; then
            if command -v python2 &> /dev/null; then
                export PYTHON=python2
            elif command -v python2.7 &> /dev/null; then
                export PYTHON=python2.7
            fi
        fi

        echo "Configuring IBEX..."
        ./waf configure --prefix="$IBEX_INSTALL_DIR" --lp-lib=soplex

        echo ""
        echo -e "${YELLOW}Building IBEX (this will take 10-20 minutes)...${NC}"
        ./waf build -j"$NUM_JOBS" -v

        echo "Installing IBEX..."
        ./waf install

        echo -e "${GREEN}IBEX build complete!${NC}"
        echo ""
    else
        echo -e "${YELLOW}IBEX already built, skipping...${NC}"
        echo ""
    fi
else
    echo -e "${YELLOW}Skipping IBEX build as requested${NC}"
    IBEX_INSTALL_DIR="${IBEX_INSTALL_DIR:-/usr/local}"
    echo "Using IBEX from: $IBEX_INSTALL_DIR"
    echo ""
fi

#===============================================================================
# Build CIRE
#===============================================================================

if [ "$SKIP_CIRE" = false ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  Building CIRE with Static Linking                         ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""

    # Get CIRE source directory (script should be in CIRE root)
    CIRE_SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    CIRE_BUILD_DIR="$BUILD_DIR/cire-build"

    mkdir -p "$CIRE_BUILD_DIR"
    cd "$CIRE_BUILD_DIR"

    echo "Configuring CIRE..."
    cmake "$CIRE_SRC_DIR" \
        -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DENABLE_LLVM_FRONTEND=ON \
        -DLLVM_STATIC_LINKING=ON \
        -DLT_LLVM_INSTALL_DIR="$LLVM_INSTALL_DIR" \
        -DIBEX_INSTALL_DIR="$IBEX_INSTALL_DIR"

    echo ""
    echo -e "${YELLOW}Building CIRE...${NC}"
    ninja -j"$NUM_JOBS" CIRE_LLVM CIRE

    echo ""
    echo "Stripping binaries..."
    strip bin/CIRE_LLVM bin/CIRE || true

    echo -e "${GREEN}CIRE build complete!${NC}"
    echo ""

    # Show binary info
    echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║                    Build Complete!                         ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Binaries built in: $CIRE_BUILD_DIR/bin"
    echo ""

    if [ -f "$CIRE_BUILD_DIR/bin/CIRE_LLVM" ]; then
        echo -e "${GREEN}CIRE_LLVM:${NC}"
        echo "  Size: $(du -h "$CIRE_BUILD_DIR/bin/CIRE_LLVM" | cut -f1)"
        echo "  Path: $CIRE_BUILD_DIR/bin/CIRE_LLVM"

        # Check dependencies
        if command -v ldd &> /dev/null; then
            echo ""
            echo -e "${BLUE}Dynamic dependencies:${NC}"
            ldd "$CIRE_BUILD_DIR/bin/CIRE_LLVM" 2>/dev/null || true
        fi
    fi

    if [ -f "$CIRE_BUILD_DIR/bin/CIRE" ]; then
        echo ""
        echo -e "${GREEN}CIRE:${NC}"
        echo "  Size: $(du -h "$CIRE_BUILD_DIR/bin/CIRE" | cut -f1)"
        echo "  Path: $CIRE_BUILD_DIR/bin/CIRE"
    fi

    echo ""
    echo -e "${GREEN}Next steps:${NC}"
    echo "  1. Test the binary: $CIRE_BUILD_DIR/bin/CIRE_LLVM --help"
    echo "  2. Add to PATH: export PATH=$CIRE_BUILD_DIR/bin:\$PATH"
    echo "  3. Or copy to system: sudo cp $CIRE_BUILD_DIR/bin/CIRE_LLVM /usr/local/bin/"

else
    echo -e "${YELLOW}Skipping CIRE build as requested${NC}"
    echo "Dependencies installed to: $INSTALL_PREFIX"
fi

echo ""
echo -e "${GREEN}Build script completed successfully!${NC}"
echo ""
