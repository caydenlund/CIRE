#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --job-name=Scan_4096_cire_abs
#SBATCH -o Scan_4096_cire_abs.log
#SBATCH -e Scan_4096_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    Scan_4096.txt \
    --debug-level 1 \
    --log-level 2 \
    --output Scan_4096_cire_noAbs.csv \
    --log-output Scan_4096_cire_noAbs.clog \
    --csv-friendly
