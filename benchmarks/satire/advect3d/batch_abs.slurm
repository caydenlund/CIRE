#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=advect3d_20_cire_abs
#SBATCH -o advect3d_20_cire_abs.log
#SBATCH -e advect3d_20_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    advect3d_20.txt \
    --debug-level 1 \
    --log-level 2 \
    --output advect3d_20_cire_noAbs.csv \
    --log-output advect3d_20_cire_noAbs.clog \
    --csv-friendly
