#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --job-name=_cire_abs
#SBATCH -o _cire_abs.log
#SBATCH -e _cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    CG_Tref_noError_K20_N20_E0.0.txt \
    --debug-level 1 \
    --log-level 2 \
    --output CG_Tref_noError_K20_N20_E0.0_cire_noAbs.csv \
    --log-output CG_Tref_noError_K20_N20_E0.0_cire_noAbs.clog \
    --csv-friendly
