# Multi-stage Dockerfile for CIRE with LLVM
# Builds CIRE, IBEX, and LLVM from scratch
# This is the base image that cire-explorer builds upon
#
# Usage:
#   docker build -t cire:latest .
#   docker run --rm cire:latest CIRE_LLVM --help

# ============================================================================
# Stage 1: Build LLVM
# ============================================================================
FROM ubuntu:22.04 AS llvm-builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    python3 \
    libz-dev \
    libncurses-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# LLVM version to build
ARG LLVM_VERSION=18.1.8

# Download and extract LLVM
RUN wget https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/llvm-project-${LLVM_VERSION}.src.tar.xz && \
    tar xf llvm-project-${LLVM_VERSION}.src.tar.xz && \
    mv llvm-project-${LLVM_VERSION}.src llvm-project && \
    rm llvm-project-${LLVM_VERSION}.src.tar.xz

# Build LLVM (optimized for size, includes clang)
RUN mkdir -p llvm-build && cd llvm-build && \
    cmake ../llvm-project/llvm \
        -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/llvm \
        -DLLVM_ENABLE_PROJECTS="clang" \
        -DLLVM_TARGETS_TO_BUILD="X86;AArch64" \
        -DLLVM_ENABLE_RTTI=ON \
        -DLLVM_BUILD_LLVM_DYLIB=OFF \
        -DBUILD_SHARED_LIBS=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_OPTIMIZED_TABLEGEN=ON \
        -DLLVM_PARALLEL_LINK_JOBS=1 \
        && \
    ninja && \
    ninja install && \
    cd .. && rm -rf llvm-build llvm-project

# ============================================================================
# Stage 2: Build IBEX
# ============================================================================
FROM ubuntu:22.04 AS ibex-builder

ENV DEBIAN_FRONTEND=noninteractive

# Install IBEX build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    bison \
    flex \
    python3 \
    python-is-python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# IBEX version
ARG IBEX_VERSION=2.8.9

# Download and build IBEX
RUN wget https://github.com/ibex-team/ibex-lib/archive/refs/tags/ibex-${IBEX_VERSION}.tar.gz && \
    tar xzf ibex-${IBEX_VERSION}.tar.gz && \
    mv ibex-lib-ibex-${IBEX_VERSION} ibex-src && \
    rm ibex-${IBEX_VERSION}.tar.gz

RUN cd ibex-src && \
    ./waf configure --prefix=/opt/ibex --lp-lib=soplex && \
    ./waf build && \
    ./waf install && \
    cd .. && rm -rf ibex-src

# ============================================================================
# Stage 3: Build CIRE
# ============================================================================
FROM ubuntu:22.04 AS cire-builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    libz-dev \
    libncurses-dev \
    libgmp-dev \
    bison \
    flex \
    && rm -rf /var/lib/apt/lists/*

# Copy LLVM and IBEX from previous stages
COPY --from=llvm-builder /opt/llvm /opt/llvm
COPY --from=ibex-builder /opt/ibex /opt/ibex

WORKDIR /workspace

# Copy CIRE source
COPY . /workspace/cire

# Build CIRE with LLVM frontend enabled
RUN cd cire && \
    mkdir -p build && \
    cd build && \
    cmake .. \
        -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCIRE_ENABLE_LLVM_FRONTEND=ON \
        -DLLVM_DIR=/opt/llvm/lib/cmake/llvm \
        -DIBEX_INSTALL_DIR=/opt/ibex \
        && \
    ninja && \
    strip CIRE CIRE_LLVM

# ============================================================================
# Stage 4: Runtime Image
# ============================================================================
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

LABEL maintainer="CIRE Team"
LABEL description="CIRE - Rigorous Floating-Point Error Analysis Tool with LLVM"
LABEL version="1.0"

# Install minimal runtime dependencies
RUN apt-get update && apt-get install -y \
    libgmp10 \
    zlib1g \
    libncurses6 \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Copy CIRE binaries
COPY --from=cire-builder /workspace/cire/build/CIRE /usr/local/bin/
COPY --from=cire-builder /workspace/cire/build/CIRE_LLVM /usr/local/bin/

# Copy LLVM tools (clang, llvm-dis, opt, etc.)
COPY --from=llvm-builder /opt/llvm/bin/clang /usr/local/bin/
COPY --from=llvm-builder /opt/llvm/bin/clang++ /usr/local/bin/
COPY --from=llvm-builder /opt/llvm/bin/llvm-as /usr/local/bin/
COPY --from=llvm-builder /opt/llvm/bin/llvm-dis /usr/local/bin/
COPY --from=llvm-builder /opt/llvm/bin/opt /usr/local/bin/
COPY --from=llvm-builder /opt/llvm/bin/llc /usr/local/bin/

# Copy LLVM libraries (needed for runtime)
COPY --from=llvm-builder /opt/llvm/lib /opt/llvm/lib

# Copy IBEX runtime libraries
COPY --from=ibex-builder /opt/ibex/lib /opt/ibex/lib

# Set library paths
ENV LD_LIBRARY_PATH=/opt/llvm/lib:/opt/ibex/lib:$LD_LIBRARY_PATH

# Update library cache
RUN ldconfig

# Set up working directory
WORKDIR /workspace

# Default command shows help
ENTRYPOINT ["/usr/local/bin/CIRE_LLVM"]
CMD ["--help"]

# ============================================================================
# Usage Examples:
# ============================================================================
# Build the image:
#   docker build -t cire:latest .
#
# Run CIRE on an LLVM IR file:
#   docker run --rm -v $(pwd):/workspace cire:latest \
#     foo.ll --domain domain.json --function foo
#
# Compile C to LLVM IR and analyze:
#   docker run --rm -v $(pwd):/workspace cire:latest \
#     bash -c "clang -S -emit-llvm foo.c -o foo.ll && \
#              CIRE_LLVM foo.ll --domain domain.json --function foo"
#
# Interactive shell:
#   docker run --rm -it -v $(pwd):/workspace --entrypoint /bin/bash cire:latest
#
# Check LLVM version:
#   docker run --rm --entrypoint clang cire:latest --version
# ============================================================================
