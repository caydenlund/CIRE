#!/bin/bash

# Docker build script for creating distributable CIRE binaries
# Builds LLVM, IBEX, and CIRE in an Ubuntu 20.04 container

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
LLVM_VERSION="${LLVM_VERSION:-21.1.7}"
IBEX_VERSION="${IBEX_VERSION:-2.8.9}"
OUTPUT_DIR="${OUTPUT_DIR:-./dist}"
BUILD_TARGET="${BUILD_TARGET:-artifacts}"
IMAGE_NAME="cire-builder"

# Parse command line arguments
EXTRACT_ONLY=false
SKIP_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --extract-only)
            EXTRACT_ONLY=true
            shift
            ;;
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        --runtime)
            BUILD_TARGET="runtime"
            shift
            ;;
        --output|-o)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --llvm-version)
            LLVM_VERSION="$2"
            shift 2
            ;;
        --ibex-version)
            IBEX_VERSION="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --extract-only        Only extract binaries from existing image"
            echo "  --skip-build          Skip Docker build, use existing image"
            echo "  --runtime             Build runtime image instead of artifacts"
            echo "  --output DIR          Output directory for binaries (default: ./dist)"
            echo "  --llvm-version VER    LLVM version to build (default: 21.1.7)"
            echo "  --ibex-version VER    IBEX version to build (default: 2.8.9)"
            echo "  --help                Show this help message"
            echo ""
            echo "Environment variables:"
            echo "  LLVM_VERSION          Override LLVM version"
            echo "  IBEX_VERSION          Override IBEX version"
            echo "  OUTPUT_DIR            Override output directory"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║          CIRE Docker Build System (Ubuntu 20.04)           ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

if [ "$SKIP_BUILD" = false ] && [ "$EXTRACT_ONLY" = false ]; then
    echo -e "${GREEN}Configuration:${NC}"
    echo "  LLVM Version: $LLVM_VERSION"
    echo "  IBEX Version: $IBEX_VERSION"
    echo "  Output Directory: $OUTPUT_DIR"
    echo "  Build Target: $BUILD_TARGET"
    echo ""

    # Check if Docker is available
    if ! command -v docker &> /dev/null; then
        echo -e "${RED}Error: Docker is not installed or not in PATH${NC}"
        exit 1
    fi

    echo -e "${GREEN}Building Docker image...${NC}"
    echo "This will take 30-60 minutes on first build (LLVM takes time)"
    echo ""

    # Build the Docker image
    docker build \
        --target "$BUILD_TARGET" \
        --build-arg LLVM_VERSION="$LLVM_VERSION" \
        --build-arg IBEX_VERSION="$IBEX_VERSION" \
        -f Dockerfile.build \
        -t "$IMAGE_NAME:latest" \
        . || {
            echo -e "${RED}Docker build failed!${NC}"
            exit 1
        }

    echo ""
    echo -e "${GREEN}Docker build completed successfully!${NC}"
    echo ""
fi

# Extract binaries if building artifacts
if [ "$BUILD_TARGET" = "artifacts" ] && [ "$EXTRACT_ONLY" = false ]; then
    echo -e "${GREEN}Extracting binaries...${NC}"

    # Create output directory
    mkdir -p "$OUTPUT_DIR"

    # Create a temporary container to extract files
    CONTAINER_ID=$(docker create "$IMAGE_NAME:latest")

    # Extract binaries
    echo "Extracting CIRE_LLVM..."
    docker cp "$CONTAINER_ID:/CIRE_LLVM" "$OUTPUT_DIR/CIRE_LLVM" || {
        echo -e "${YELLOW}Warning: Could not extract CIRE_LLVM${NC}"
    }

    echo "Extracting CIRE..."
    docker cp "$CONTAINER_ID:/CIRE" "$OUTPUT_DIR/CIRE" || {
        echo -e "${YELLOW}Warning: Could not extract CIRE${NC}"
    }

    # Remove temporary container
    docker rm "$CONTAINER_ID" > /dev/null

    # Make binaries executable
    chmod +x "$OUTPUT_DIR"/* 2>/dev/null || true

    echo ""
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                    Build Complete!                         ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Binaries extracted to: $OUTPUT_DIR"
    echo ""

    # Show binary info
    if [ -f "$OUTPUT_DIR/CIRE_LLVM" ]; then
        echo -e "${BLUE}CIRE_LLVM:${NC}"
        echo "  Size: $(du -h "$OUTPUT_DIR/CIRE_LLVM" | cut -f1)"
        echo "  Path: $OUTPUT_DIR/CIRE_LLVM"

        # Check dependencies
        if command -v ldd &> /dev/null; then
            echo ""
            echo -e "${BLUE}Dynamic dependencies (should be minimal):${NC}"
            ldd "$OUTPUT_DIR/CIRE_LLVM" 2>/dev/null | grep -v "linux-vdso\|ld-linux" || true
        fi
    fi

    if [ -f "$OUTPUT_DIR/CIRE" ]; then
        echo ""
        echo -e "${BLUE}CIRE:${NC}"
        echo "  Size: $(du -h "$OUTPUT_DIR/CIRE" | cut -f1)"
        echo "  Path: $OUTPUT_DIR/CIRE"
    fi

    echo ""
    echo -e "${GREEN}Ready for distribution!${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Test the binary: $OUTPUT_DIR/CIRE_LLVM --help"
    echo "  2. Create release package: tar czf cire-linux-x86_64.tar.gz -C $OUTPUT_DIR ."
    echo "  3. Upload to GitHub releases or distribute as needed"

elif [ "$BUILD_TARGET" = "runtime" ]; then
    echo -e "${GREEN}Runtime image built successfully!${NC}"
    echo ""
    echo "To run CIRE in a container:"
    echo "  docker run --rm -v \$(pwd):/workspace $IMAGE_NAME:latest --help"
    echo ""
    echo "To run with a file:"
    echo "  docker run --rm -v \$(pwd):/workspace $IMAGE_NAME:latest /workspace/your_file.ll"
fi

echo ""
