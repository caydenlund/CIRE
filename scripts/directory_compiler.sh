#!/bin/bash

for file in /home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_c/*.c; do
    /home/tanmay/Documents/Tools/llvm-clone/llvm/build-release/bin/clang -S -emit-llvm -O1 -ffast-math "$file" -o /home/tanmay/Documents/Tools/CIRE/benchmarks/fpbench_ll_fast_math/$(basename "$file" .c).ll
done
