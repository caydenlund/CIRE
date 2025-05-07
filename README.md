# CIRE: C++ Incremental rigorous error-analyser

CIRE is similar to [SATIRE](https://github.com/arnabd88/Satire) but in C++.
This project was meant to provide an error analysis tool and library for C/C++ programs.
Ideally it should have all the capabilities of Satire and more.


# Dependencies

CIRE requires the following softwares installed on your system.
The tool will not build unless these are installed.

* ibex-lib >= 2.8.9 [Install Notes](#ibex-installation)
  * [Github](https://github.com/ibex-team/ibex-lib)
  * [Original Installation instructions](http://ibex-team.github.io/ibex-lib/install.html)
* python2.x >= 2.7 (Ibex scripts are currently not compatible with python3)
* g++ >= 13 (because of IBEX)
* gcc >= 13 (because of IBEX)
* bison
* flex
* cmake

### If you want to use the LLVM frontend

* LLVM >= 16

This version is necessary since the frontend uses the new pass manager by default.
Use gcc (version 13) to build LLVM to ensure standard libraries are compatible with CIRE and IBEX.
LLVM 16 has been tested to work.

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

The CMakeLists.txt file is configured to build the library and the executable in the build directory.
Make sure the shared library files are in your PATH. If not, set the LD_LIBRARY_PATH environment variable to where your
shared library files are located.
Then run the following:

```bash
mkdir build-debug
cd build-debug
cmake ..
make
```

## Building the LLVM frontend

To build the LLVM frontend, you need to have a build of LLVM 16 or newer on your system.
Pass `-DENABLE_LLVM_FRONTEND=ON` to cmake and set `LT_LLVM_INSTALL_DIR` to the directory where LLVM is installed.

```bash
cmake -DENABLE_LLVM_FRONTEND=ON                               \
      -DLT_LLVM_INSTALL_DIR=<path to LLVM install directory>  \
      ..
```

## Targets

### CIRE

Uses the SATIRE DSL frontend.
```bash
make CIRE
```


# Usage

The executable is located in the build-debug directory. Run the following to see CIRE run on an example file

```bash
LD_LIBRARY_PATH=/usr/local/lib
CIRE ./benchmarks/addition/addition.txt
```

## LLVM Frontend

There are two ways to utilize the LLVM Frontend: as an LLVM pass or as a standalone tool.
Make sure you run `-O1` or higher to generate LLVM IR to simplify the function, removing all memory operations.

```bash
clang -S -emit-llvm -O1 <path/to/c/file>
```

### As an LLVM pass

```bash
<path/to/opt> -load-pass-plugin=<path/to/libCIRE_LLVM.so> -passes="cire" --function=<function_name> --input=<path/to/.txt/file> --disable-output <path/to/llvm/ir/file>
```

### As a standalone tool

```bash
CIRE_LLVM <path/to/llvm/ir/file> --function=<function_name> --input=<path/to/.txt/file>
```

## Writing LLVM tests

Steps to comply to make CIRE test script work
- The test file should have extension .ll
- The input file should be named "test_input.txt"
- The function argument names in the .ll and .txt file should match
- If a there is no corresponding input file, default input values will be assumed.
