#!/bin/bash

git pull
cd build-debug
module load gcc/13.1.0
module load cmake/3.26.0
cmake -DLT_LLVM_INSTALL_DIR=/uufs/chpc.utah.edu/common/home/u1260704/Tools/llvm-clone/llvm/build-release -DENABLE_LLVM_FRONTEND=ON ..
make CIRE_LLVM
make CIRE