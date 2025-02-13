#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=ccsd_type1_var_0_cire_abs
#SBATCH -o ccsd_type1_var_0_cire_abs.log
#SBATCH -e ccsd_type1_var_0_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    ccsd_type1_var_0.txt \
    --debug-level 1 \
    --log-level 2 \
    --output ccsd_type1_var_0_cire_noAbs.csv \
    --log-output ccsd_type1_var_0_cire_noAbs.clog \
    --csv-friendly
