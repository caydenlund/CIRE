#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=wave2d1_t16_cire_abs
#SBATCH -o wave2d1_t16_cire_abs.log
#SBATCH -e wave2d1_t16_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=96:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

# No abstraction
../../../build-debug/bin/CIRE \
    wave2d1_t16.txt \
    --debug-level 1 \
    --log-level 2 \
    --output wave2d1_t16_cire_noAbs.csv \
    --log-output wave2d1_t16_cire_noAbs.clog \
    --csv-friendly
