#!/bin/bash
#SBATCH --account=soc-kp
#SBATCH --partition=soc-kp
#SBATCH --reservation=DAT-u1260704
#SBATCH --job-name=benchmarks_abs
#SBATCH -o benchmarks-abs.log
#SBATCH -e benchmarks-abs.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=72:00:00
#SBATCH --export=ALL
#SBATCH --qos=soc-kp


DIRS="
	poisson2d0\
	poisson2d1_t32\
	poisson2d2_t32\
	convecdiff2d0\
	convecdiff2d1_t32\
	convecdiff2d2_t32\
	heat1d_t32\
	heat2d0_t32\
	heat2d1_t32\
	heat2d2_t32\
	fdtd1d_t64\
	lorentz20\
	lorentz30\
	lorentz40\
	lorentz70\
	matmul64\
	matmul128\
	FFT_1024\
	FFT_4096pt\
	dqmom\
	MD\
	chainSum\
	poly-eval\
	horner\
	reduction\
	Scan_1024\
	Scan_4096\
	CG_Arc\
	CG_EX5\
	CG_Pores\
	CG_Tref\
	advect3d\
  var_ccsd_type0_0\
  var_ccsd_type0_1\
  var_ccsd_type1_0\
  var_ccsd_type1_1\
  var_ccsd_type2_0\
  var_ccsd_type2_1\
  ccsd_type0_1\
  ccsd_type1_0\
  ccsd_type1_1\
  ccsd_type2_0\
  ccsd_type2_1\
	wave2d1
	"

set -x


#find $GPHOME -name "*generated*"

if [[ -f Results.txt ]]
then
  echo "Removing old results file"
  rm -rf Results.txt
fi

for d in $DIRS
do
  echo $d
  cd $d
  echo "executing batch scipt for " $d
  bash batch_abs.slurm
  python3 ../collect_results.py $d
  echo $PWD
  cat Results.txt >> ../Results.txt
  cd ..
done

echo "Open Results.txt for summary of the evaluation"

exit