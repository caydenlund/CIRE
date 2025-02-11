#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --job-name=heat2d2_t32_cire_abs
#SBATCH -o heat2d2_t32_cire_abs.log
#SBATCH -e heat2d2_t32_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    heat2d2_t32.txt \
    --debug-level 1 \
    --log-level 2 \
    --output heat2d2_t32_cire_noAbs.csv \
    --log-output heat2d2_t32_cire_noAbs.clog \
    --csv-friendly
