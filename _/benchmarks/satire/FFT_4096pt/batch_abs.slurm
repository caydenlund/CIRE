#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=fft_4096pt_cire_abs
#SBATCH -o fft_4096pt_cire_abs.log
#SBATCH -e fft_4096pt_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=96:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

# No abstraction
../../../build-debug/bin/CIRE \
    fft_4096pt.txt \
    --debug-level 1 \
    --log-level 2 \
    --output fft_4096pt_cire_noAbs.csv \
    --log-output fft_4096pt_cire_noAbs.clog \
    --csv-friendly
