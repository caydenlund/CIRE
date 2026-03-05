# CIRE: C++ Incremental Rigorous Error Analyser

CIRE is a rigorous floating-point error analysis tool for C/C++ programs.
It computes sound upper bounds on numerical errors using interval arithmetic and automatic differentiation.
Similar to [SATIRE](https://github.com/arnabd88/Satire), but implemented in C++ with additional features.

## Features

- **Two frontends**: Analyze LLVM IR (from C/C++) or SATIRE DSL programs
- **Rigorous bounds**: Sound upper bounds on floating-point errors using IBEX interval arithmetic
- **Automatic differentiation**: Efficient error propagation through computation graphs
- **Docker support**: Build and run without installing dependencies locally
- **Multiple outputs**: Absolute error, relative error, ULPs, and witness inputs

## Quick Start (Docker)

The easiest way to use CIRE is with Docker:

```bash
# Build the Docker image
./docker-build.sh

# Run on a C file
docker run --rm -v $(pwd):/workspace cire:latest bash -c \
  "clang -S -emit-llvm -O0 myfile.c -o myfile.ll && \
   CIRE_LLVM myfile.ll --domain domain.json --function myfunction"
```

See [DOCKER.md](DOCKER.md) for detailed Docker usage and publishing instructions.

## Build Dependencies

CIRE requires the following software installed on your system:

* **ibex-lib >= 2.8.9** - [Install Notes](#ibex-installation)
  * [GitHub](https://github.com/ibex-team/ibex-lib)
  * [Installation instructions](http://ibex-team.github.io/ibex-lib/install.html)
* **CMake >= 3.20**
* **g++ >= 13** (for IBEX compatibility)
* **gcc >= 13** (for IBEX compatibility)
* **Bison**
* **Flex**
* **Python 3** (for IBEX build scripts)

### Optional: LLVM frontend

* **LLVM >= 16**

Required to build `CIRE_LLVM` for analyzing LLVM IR from C/C++ programs.
Use GCC 13+ to build LLVM to ensure standard libraries are compatible with CIRE and IBEX.
LLVM 16, 17, and 18 have been tested to work.

## IBEX installation

### Linux and MacOS

Use `wget` command below or download `ibex-lib-ibex-2.8.9.tar.gz` (the tar file) from
[the GitHub release page](https://github.com/ibex-team/ibex-lib/releases/tag/ibex-2.8.9).

```bash
wget https://github.com/ibex-team/ibex-lib/archive/refs/tags/ibex-2.8.9.tar.gz
```

#### Installing IBEX through waflib (RECOMMENDED)

```bash
tar xvfz ibex-2.8.9.tgz
cd ibex-2.8.9
./waf configure --lp-lib=soplex [--enable-shared]
./waf install
```

**In case the waflib configuration fails** for some reason, you may need to install one or more of the libraries listed below, 
that are archived in IBEX, manually and use the CMake build instead:
* Mathlib
* GAOL
* Soplex (Most likely to fail)

Both waflib and CMake attempt to build each library automatically if it is not detected on your system so you only need
to build the libraries that the CMake system fails to build.
So after unzipping ibex tar ball and moving into the unzipped folder,

#### Installing Soplex

```bash
cd lp_lib_wrapper/soplex/3rd
tar soplex-4.0.2.tar
cd soplex-4.0.2
mkdir build && cd build
cmake .. -DLP_LIB=soplex
make -j16 USRCPPFLAGS=-fPIC
sudo make install
```

#### Installing IBEX through CMake

```bash
mkdir build && cd build
cmake -DLP_LIB=soplex ..
make -j16
sudo make install
```


# Building CIRE

## Docker Build (Recommended)

The easiest way to build CIRE with all dependencies:

```bash
./docker-build.sh
```

This builds CIRE, LLVM, and IBEX in a multi-stage Docker container.
The final image (~600MB) includes:
- `CIRE` - SATIRE frontend
- `CIRE_LLVM` - LLVM IR frontend
- `clang` - C/C++ compiler
- LLVM tools (`llvm-dis`, `opt`, etc.)

See [DOCKER.md](DOCKER.md) for advanced Docker usage, publishing, and troubleshooting.

## Manual Build

If you have IBEX installed on your system:

### 1. Basic build (SATIRE frontend only)

```bash
mkdir build
cd build
cmake .. -DIBEX_INSTALL_DIR=/path/to/ibex
make
```

This builds the `CIRE` binary for analyzing SATIRE DSL programs.

### 2. Build with LLVM frontend

```bash
mkdir build
cd build
cmake .. \
  -DCIRE_ENABLE_LLVM_FRONTEND=ON \
  -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm \
  -DIBEX_INSTALL_DIR=/path/to/ibex
make
```

This builds both `CIRE` and `CIRE_LLVM` binaries.

### 3. Customize build type

```bash
# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release -DIBEX_INSTALL_DIR=/path/to/ibex

# Debug build (with debug symbols)
cmake .. -DCMAKE_BUILD_TYPE=Debug -DIBEX_INSTALL_DIR=/path/to/ibex
```

## Build Targets

- `CIRE` - SATIRE frontend (always built)
- `CIRE_LLVM` - LLVM IR frontend (requires `-DCIRE_ENABLE_LLVM_FRONTEND=ON`)
- `cire_core` - Core library (built automatically)
- `cire_llvm_frontend` - LLVM frontend library (built when LLVM is enabled)


# Usage

CIRE has two executables:
- `CIRE` - For analyzing SATIRE DSL programs
- `CIRE_LLVM` - For analyzing LLVM IR from C/C++ programs

## Using CIRE_LLVM (LLVM IR Frontend)

### Step 1: Compile C/C++ to LLVM IR

```bash
clang -S -emit-llvm -O1 myfile.c -o myfile.ll
```

**Note**: Use at least `-O1` to avoid memory allocations.

### Step 2: Create a domain file (optional)

Create a JSON file specifying the input ranges for your function arguments:

```json
{
  "x": [-1000000, 1000000],
  "y": [0, 100]
}
```

### Step 3: Run CIRE

If you created a JSON file specifying the input ranges:

```bash
./build/CIRE_LLVM myfile.ll --domain domain.json --function myfunction
```

Otherwise, you can supply them as command-line arguments:


```bash
./build/CIRE_LLVM myfile.ll --default-domain [-1e6,1e6] -d y=[0,100] --function myfunction
```


### Full example

```bash
# Create a simple C file
cat > cube.c << 'EOF'
double cube(double x) {
    return x * x * x;
}
EOF

# Create domain file
cat > domain.json << 'EOF'
{
  "x": [-1000000, 1000000]
}
EOF

# Compile to LLVM IR
clang -S -emit-llvm -O0 cube.c -o cube.ll

# Run CIRE
./build/CIRE_LLVM cube.ll --domain domain.json --function cube

# Or, omitting the domain file:
./build/CIRE_LLVM cube.ll -d x=[-1e6,1e6] --function cube
```

### Expected output

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

## Using CIRE (SATIRE Frontend)

For SATIRE DSL programs:

```bash
./build/CIRE program.txt --domain domain.json
```

The SATIRE frontend uses a custom DSL for specifying computations.
See the `_/benchmarks` directory for examples.

## Command-line Options

### CIRE_LLVM

```bash
CIRE_LLVM <input.ll> --domain <domain.json> --function <name> [OPTIONS]

Required:
  input.ll              LLVM IR file (.ll or .bc)
  --domain FILE         JSON file specifying input variable intervals
  --function NAME       Name of function to analyze

Optional:
  --emit-graph         Dump computation graph as DOT file
  --emit-expr          Dump error expression AST
  -v, --verbose        Verbose output
  -h, --help           Show help message
```

### CIRE

```bash
CIRE <input.txt> --domain <domain.json> [OPTIONS]

Required:
  input.txt            SATIRE source file
  --domain FILE        JSON file specifying input variable intervals

Optional:
  --emit-graph        Dump computation graph as DOT file
  --emit-expr         Dump error expression AST
  -v, --verbose       Verbose output
  -h, --help          Show help message
```

## Docker Usage

See [DOCKER.md](DOCKER.md) for detailed Docker usage examples, or try the example script:

```bash
./examples/docker-example.sh
```

## Testing

Run the SATIRE benchmark tests:

```bash
./test_satire_benchmarks.sh          # Normal mode (10s timeout)
./test_satire_benchmarks.sh --fast   # Fast mode (1s timeout)
./test_satire_benchmarks.sh --extended  # Extended mode (60s timeout)
```
