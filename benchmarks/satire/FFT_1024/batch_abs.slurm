#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=fft_1024pt_cire_abs
#SBATCH -o fft_1024pt_cire_abs.log
#SBATCH -e fft_1024pt_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

# Abs window : (15,25)
../../../build-debug/bin/CIRE \
    fft_1024pt_sat.txt \
    --abstraction \
    --min-depth 15 \
    --max-depth 25 \
    --debug-level 1 \
    --log-level 2 \
    --output fft_1024pt_cire_15_25.csv \
    --log-output fft_1024pt_cire_15_25.clog \
    --csv-friendly

# No abstraction
../../../build-debug/bin/CIRE \
    fft_1024pt_sat.txt \
    --debug-level 1 \
    --log-level 2 \
    --output fft_1024pt_cire_noAbs.csv \
    --log-output fft_1024pt_cire_noAbs.clog \
    --csv-friendly
