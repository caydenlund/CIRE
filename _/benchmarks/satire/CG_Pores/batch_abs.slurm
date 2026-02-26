#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=CG_Pores_cire_abs
#SBATCH -o CG_Pores_cire_abs.log
#SBATCH -e CG_Pores_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=96:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

# No abstraction
../../../build-debug/bin/CIRE \
    CG_Pores_noError_K20_N30_E0.0.txt \
    --debug-level 1 \
    --log-level 2 \
    --output CG_Pores_noError_K20_N30_E0.0_cire_noAbs.csv \
    --log-output CG_Pores_noError_K20_N30_E0.0_cire_noAbs.clog \
    --csv-friendly
