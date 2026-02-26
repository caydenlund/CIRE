#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=MD_2_10_cire_abs
#SBATCH -o MD_2_10_cire_abs.log
#SBATCH -e MD_2_10_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=96:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

# No abstraction
../../../build-debug/bin/CIRE \
    MD_2_10_step10.txt \
    --debug-level 1 \
    --log-level 2 \
    --output MD_2_10_step10_cire_noAbs.csv \
    --log-output MD_2_10_step10_cire_noAbs.clog \
    --csv-friendly

# No abstraction
../../../build-debug/bin/CIRE \
    MD_2_10-err.txt \
    --debug-level 1 \
    --log-level 2 \
    --output MD_2_10-err_cire_noAbs.csv \
    --log-output MD_2_10-err_cire_noAbs.clog \
    --csv-friendly