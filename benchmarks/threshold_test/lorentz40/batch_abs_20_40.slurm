#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704-2
#SBATCH --job-name=lorentz_40_cire_20_40
#SBATCH -o lorentz_40_cire_20_40.log
#SBATCH -e lorentz_40_cire_20_40.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=96:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 10 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 20 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 40 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 80 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 200 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 400 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 800 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    -op 2000 \
    --csv-friendly

../../../build-debug/bin/CIRE \
    lorentz_40.txt \
    --abstraction \
    --min-depth 20 \
    --max-depth 40 \
    --debug-level 3 \
    --log-level 4 \
    --output lorentz_40_cire_20_40.csv \
    --log-output lorentz_40_cire_20_40.clog \
    --csv-friendly