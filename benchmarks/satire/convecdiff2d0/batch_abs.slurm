#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=convecdiff2d0_t32_cire_abs
#SBATCH -o convecdiff2d0_t32_cire_abs.log
#SBATCH -e convecdiff2d0_t32_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=96:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

# No abstraction
../../../build-debug/bin/CIRE \
    convecdiff2d0_t32.txt \
    --debug-level 1 \
    --log-level 2 \
    --output convecdiff2d0_t32_cire_noAbs.csv \
    --log-output convecdiff2d0_t32_cire_noAbs.clog \
    --csv-friendly
