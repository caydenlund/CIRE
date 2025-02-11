#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --job-name=CG_ex5_cire_abs
#SBATCH -o CG_ex5_cire_abs.log
#SBATCH -e CG_ex5_cire_abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    CG_ex5_noError_K20_N27_E0.0.txt \
    --debug-level 1 \
    --log-level 2 \
    --output CG_ex5_noError_K20_N27_E0.0_cire_noAbs.csv \
    --log-output CG_ex5_noError_K20_N27_E0.0_cire_noAbs.clog \
    --csv-friendly

../../../build-debug/bin/CIRE \
    CG_ex5_InError_K20_N27_E0.1.txt \
    --debug-level 1 \
    --log-level 2 \
    --output CG_ex5_InError_K20_N27_E0.1_cire_noAbs.csv \
    --log-output CG_ex5_InError_K20_N27_E0.1_cire_noAbs.clog \
    --csv-friendly

../../../build-debug/bin/CIRE \
    CG_ex5_InError_K20_N27_E0.txt \
    --debug-level 1 \
    --log-level 2 \
    --output CG_ex5_InError_K20_N27_E0_cire_noAbs.csv \
    --log-output CG_ex5_InError_K20_N27_E0_cire_noAbs.clog \
    --csv-friendly