#!/bin/bash
# Build script for CIRE Docker image

set -e

# Default values
IMAGE_NAME="cire"
IMAGE_TAG="latest"
REGISTRY=""
PUSH=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --name)
            IMAGE_NAME="$2"
            shift 2
            ;;
        --tag)
            IMAGE_TAG="$2"
            shift 2
            ;;
        --registry)
            REGISTRY="$2"
            shift 2
            ;;
        --push)
            PUSH=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --name NAME        Docker image name (default: cire)"
            echo "  --tag TAG          Docker image tag (default: latest)"
            echo "  --registry REG     Registry to push to (e.g., ghcr.io/username)"
            echo "  --push             Push image after building"
            echo "  --help             Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0"
            echo "  $0 --tag v1.0"
            echo "  $0 --registry registry.example.com/myorg --tag v1.0 --push"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Construct full image name
if [ -n "$REGISTRY" ]; then
    FULL_IMAGE="${REGISTRY}/${IMAGE_NAME}:${IMAGE_TAG}"
else
    FULL_IMAGE="${IMAGE_NAME}:${IMAGE_TAG}"
fi

echo "========================================="
echo "Building CIRE Docker Image"
echo "========================================="
echo "Image: $FULL_IMAGE"
echo "Push: $PUSH"
echo ""

# Build the image
echo "Building image..."
docker build -t "$FULL_IMAGE" .

echo ""
echo "Build complete!"
echo "Image: $FULL_IMAGE"
echo ""

# Show image size
echo "Image size:"
docker images "$FULL_IMAGE" --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}"
echo ""

# Push if requested
if [ "$PUSH" = true ]; then
    echo "Pushing to registry..."
    docker push "$FULL_IMAGE"
    echo "Push complete!"
fi

echo ""
echo "========================================="
echo "Usage examples:"
echo "========================================="
echo ""
echo "Run CIRE_LLVM:"
echo "  docker run --rm -v \$(pwd):/workspace $FULL_IMAGE foo.ll --domain domain.json --function foo"
echo ""
echo "Interactive shell:"
echo "  docker run --rm -it -v \$(pwd):/workspace --entrypoint /bin/bash $FULL_IMAGE"
echo ""
echo "Compile and analyze C code:"
echo "  docker run --rm -v \$(pwd):/workspace $FULL_IMAGE bash -c \\"
echo "    'clang -S -emit-llvm foo.c -o foo.ll && CIRE_LLVM foo.ll --domain domain.json --function foo'"
echo ""
