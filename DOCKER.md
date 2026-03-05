# CIRE Docker Container

This document describes how to build, publish, and use the CIRE Docker container.

## Overview

The CIRE Docker container includes:
- **CIRE**: Error analysis tools (both SATIRE and LLVM frontends)
- **LLVM Toolchain**: `clang`, `llvm-dis`, `opt`, and other LLVM tools
- All necessary runtime libraries

This allows you to analyze C/C++ programs and LLVM IR files without installing dependencies.

## Quick Start

### Pull from Registry

```bash
docker pull caydenlund/cire:latest
```

### Build Locally

```bash
# Simple build
docker build -t cire:latest .

# Or use the build script
./docker-build.sh
```

## Usage Examples

### 1. Analyze LLVM IR File

```bash
docker run --rm -v $(pwd):/workspace cire:latest \
  foo.ll --domain domain.json --function foo
```

### 2. Compile C to LLVM IR and Analyze

```bash
docker run --rm -v $(pwd):/workspace cire:latest bash -c \
  "clang -S -emit-llvm foo.c -o foo.ll && \
   CIRE_LLVM foo.ll --domain domain.json --function foo"
```

### 3. Interactive Shell

```bash
docker run --rm -it -v $(pwd):/workspace \
  --entrypoint /bin/bash cire:latest
```

Inside the container, you can use:
- `CIRE_LLVM` - Analyze LLVM IR
- `CIRE` - Analyze SATIRE programs
- `clang` - Compile C/C++ to LLVM IR
- `llvm-dis` - Disassemble LLVM bitcode
- `opt` - LLVM optimizer

### 4. Example: Full Workflow

Create a simple C file:

```c
// cube.c
double cube(double x) {
    return x * x * x;
}
```

Create a domain file:

```json
# domain.json
{
  "x": [-1000000, 1000000]
}
```

Run the analysis:

```bash
docker run --rm -v $(pwd):/workspace cire:latest bash -c "
  clang -S -emit-llvm -O0 cube.c -o cube.ll && \
  CIRE_LLVM cube.ll --domain domain.json --function cube
"
```

Expected output:

```
========================================
CIRE Error Analysis Report
========================================

Computation Graph:
  Nodes: 3
  Outputs: 1

Error Analysis Results:
  Absolute Error Bound: 2.2204e+02
  Relative Error: 2.221e-16
  Error in ULPs: 1.74
  (1 ULP at output ≈ 1.280e+02)

  Status: Sound upper bound (not necessarily tight)

  Witness Input (achieving worst case):
    x = -1.0000e+06

  Output at witness: -1.0000e+18

========================================
```

## Building and Publishing

### Build Script Options

```bash
./docker-build.sh --help
```

Options:
- `--name NAME`: Docker image name (default: `cire`)
- `--tag TAG`: Docker image tag (default: `latest`)
- `--registry REG`: Registry URL (e.g., `ghcr.io/myorg`)
- `--push`: Push to registry after building

### Examples

Build and tag as `v1.0`:

```bash
./docker-build.sh --tag v1.0
```

Build and push to a container registry:

```bash
./docker-build.sh \
  --registry YOUR_REGISTRY/YOUR_USERNAME \
  --tag latest \
  --push
```

Build multiple tags:

```bash
./docker-build.sh --tag v1.0.0
./docker-build.sh --tag v1.0
./docker-build.sh --tag latest
```

## Advanced Usage

### Custom LLVM/IBEX Versions

Edit the `Dockerfile` and change the `ARG` lines:

```dockerfile
ARG LLVM_VERSION=18.1.8
ARG IBEX_VERSION=2.8.9
```

Then rebuild:

```bash
docker build --build-arg LLVM_VERSION=19.1.0 -t cire:llvm19 .
```

### Extracting Binaries

To extract the compiled binaries without the full runtime:

```bash
docker build --target cire-builder -t cire-builder .
docker create --name temp cire-builder
docker cp temp:/workspace/cire/build/CIRE_LLVM ./
docker cp temp:/workspace/cire/build/CIRE ./
docker rm temp
```

## Container Details

### Included Tools

- `CIRE_LLVM` - CIRE with LLVM IR frontend
- `CIRE` - CIRE with SATIRE frontend
- `clang` - C/C++ compiler
- `llvm-as` - LLVM assembler
- `llvm-dis` - LLVM disassembler
- `opt` - LLVM optimizer
- `llc` - LLVM static compiler
