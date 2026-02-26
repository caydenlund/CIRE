#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704-2
#SBATCH --job-name=Scan_1024_cire_noAbs
#SBATCH -o Scan_1024_cire_noAbs.log
#SBATCH -e Scan_1024_cire_noAbs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=96:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 10 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 20 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 40 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 80 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 200 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 400 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 800 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    -op 2000 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    Scan_1024.txt \
    --debug-level 3 \
    --log-level 4 \
    --output Scan_1024_cire_.csv \
    --log-output Scan_1024_cire_.clog \
    --csv-friendly

