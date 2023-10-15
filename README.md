# CIRE: C++ Incremental rigorous error-analyser

CIRE is similar to [SATIRE](<add link to repo>) but in C++. This project was meant to provide an error analysis tool and
library for C/C++ programs. Ideally it should have all the capabilities of Satire and
more.

# Dependencies

Cire requires the following softwares installed on your system

* ibex-lib > 2.8.9
  * [Github](https://github.com/ibex-team/ibex-lib)
  * [Installation instructions](http://ibex-team.github.io/ibex-lib/install.html)
* python2.x > 2.7 (Ibex scripts are currently not compatible with python3)
* g++
* gcc
* bison
* flex
* cmake

# Building

The CMakeLists.txt file is configured to build the library and the executable in the build directory.
Make sure the shared library files are in your PATH. If not, set the LD_LIBRARY_PATH environment variable to where your
shared library files are located.
Run the following

```cmake
mkdir build-debug
cmake CIRE
```

# Usage

The executable is located in the build-debug directory. Run the following to see CIRE run on an example file

```bash
LD_LIBRARY_PATH=/usr/local/lib
./build-debug/CIRE ./benchmarks/addition/addition.txt
```
