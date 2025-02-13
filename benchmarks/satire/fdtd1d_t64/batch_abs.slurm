#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=fdtd1d_64_cire_abs
#SBATCH -o fdtd1d_64_cire_abs.log
#SBATCH -e fdtd1d_64_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    fdtd1d_64.txt \
    --debug-level 1 \
    --log-level 2 \
    --output fdtd1d_64_cire_noAbs.csv \
    --log-output fdtd1d_64_cire_noAbs.clog \
    --csv-friendly
