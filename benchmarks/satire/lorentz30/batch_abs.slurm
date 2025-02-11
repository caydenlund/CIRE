#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --job-name=lorentz_30_cire_abs
#SBATCH -o lorentz_30_cire_abs.log
#SBATCH -e lorentz_30_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    lorentz_30_sat.txt \
    --debug-level 1 \
    --log-level 2 \
    --output lorentz_30_cire_noAbs.csv \
    --log-output lorentz_30_cire_noAbs.clog \
    --csv-friendly
