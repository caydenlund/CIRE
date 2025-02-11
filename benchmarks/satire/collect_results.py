

import sys
import glob

fout = open("Results.txt", 'w')

configs = ['noAbs',
		   # '10_20',
		   # '15_25',
		   # '20_40'
		   ]
Message = {'noAbs' : 'Without Abstraction ', \
		   # '10_20' : 'Abstraction window (10,20) ',\
		   # '15_25' : 'Abstraction window (15,25) ',\
		   # '20_40' : 'Abstraction window (20,40) '
		   }

BenchmarkNames = {
				  'chainSum' : 'Serial Summation', \
				  'dqmom' : 'Direct Quadrature Method of Moments', \
				  'poisson2d0' : 'P0', \
				  'poisson2d1_t32' : 'P1', \
				  'poisson2d2_t32' : 'P2', \
				  'convecdiff2d0' : 'C0', \
				  'convecdiff2d1_t32' : 'C1', \
				  'convecdiff2d2_t32' : 'C2', \
				  'heat1d_t32' : 'H1D', \
				  'heat2d0_t32' : 'H0', \
				  'heat2d1_t32' : 'H1', \
				  'heat2d2_t32' : 'H2', \
				  'advect3d' : 'Advection3D', \
				  'fdtd1d_t64' : 'FDTD', \
				  'lorentz20' : 'lorenz20', \
				  'lorentz30' : 'lorenz30', \
				  'lorentz40' : 'lorenz40', \
				  'lorentz70' : 'lorenz70', \
				  'matmul64' : 'Matrix Multiplication - 64x64', \
				  'matmul128' : 'Matrix Multiplication - 128x128', \
				  'MD' : 'Molecular Dynamics', \
				  'FFT_1024' : 'FFT_1024', \
				  'FFT_4096pt' : 'FFT_4096', \
				  'Scan_1024' : 'Scan_1024(Prefix sum)', \
				  'Scan_4096' : 'Scan_4096(Prefix Sum)', \
				  'CG_Arc' : 'Conjugate gradient (ARC130)', \
				  'CG_EX5' : 'Conjugate gradient (EX5)', \
				  'CG_Pores' : 'COnjugate gradient (Pores)', \
				  'CG_Tref' : 'COnjugate gradient (Tref)', \
				  'varccsd_type0_0' : 'Tensor Contraction-type01', \
				  'varccsd_type0_1' : 'Tensor Contraction-type01', \
				  'varccsd_type1_0' : 'Tensor Contraction-type10', \
				  'varccsd_type1_1' : 'Tensor Contraction-type11', \
				  'var_ccsd_type2_0' : 'Tensor Contraction-type20', \
				  'var_ccsd_type2_1' : 'Tensor Contraction-type21', \
				  'ccsd_type0_1' : 'Tensor Contraction-type01 (Degenerate Intervals)', \
				  'ccsd_type1_0' : 'Tensor Contraction-type10 (Degenerate Intervals)', \
				  'ccsd_type1_1' : 'Tensor Contraction-type11 (Degenerate Intervals)', \
				  'ccsd_type2_0' : 'Tensor Contraction-type20 (Degenerate Intervals)', \
				  'ccsd_type2_1' : 'Tensor Contraction-type21 (Degenerate Intervals)', \
				  'poly-eval' : 'Polynomial Evaluation', \
				  'reduction' : 'Reduction', \
				  'Scan_1024' : 'Scan Sum 1024 points', \
				  'Scan_4096' : 'Scan Sum 4096 points', \
				  'wave2d1' : 'Wave sim',
	}

test_name = sys.argv[1]
print(test_name)
fout.write(test_name+"\n")
file_list = list(glob.iglob('*'))

clog_dict = dict()
csv_dict = dict()
print(file_list)
# clog_dict['10_20'] = list(filter( lambda x: 'clog' in x and '10_20' in x, file_list))
# clog_dict['15_25'] = list(filter( lambda x: 'clog' in x and '15_25' in x, file_list))
# clog_dict['20_40'] = list(filter( lambda x: 'clog' in x and '20_40' in x, file_list))
clog_dict['noAbs'] = list(filter( lambda x: 'clog' in x and 'noAbs' in x, file_list))
print(clog_dict)

# csv_dict['10_20'] = list(filter(lambda x: 'csv' in x and '10_20' in x , file_list))
# csv_dict['15_25'] = list(filter(lambda x: 'csv' in x and '15_25' in x , file_list))
# csv_dict['20_40'] = list(filter(lambda x: 'csv' in x and '20_40' in x , file_list))
csv_dict['noAbs'] = list(filter(lambda x: 'csv' in x and 'noAbs' in x , file_list))
#print(clog_dict)
BenchName = BenchmarkNames.get(test_name, test_name)
print("****** Benchmark :", BenchName, "**************")
fout.write("****** Benchmark : {bench} **************\n".format(bench=BenchName))
for conf in configs:
	if (len(clog_dict[conf]) == 0):
		print(Message[conf], "did not execute \n")
		fout.write("{message} -- FAILED to execute \n".format(message=Message[conf]))
	else:
		clogname = clog_dict[conf][0]
		csvname = csv_dict[conf][0]
		#print(clogname, csvname)
		logfile = open(clogname, 'r').read().splitlines()
		csvfile = open(csvname, 'r').read().splitlines()
		fout.write("{message} -- SUCCESSFULLY executed \n".format(message=Message[conf]))
		# AST_DEPTH = list(filter(lambda x: "AST_DEPTH" in x, logfile))
		# ABSOLUTE_ERROR = list(filter(lambda x: "ABSOLUTE_ERROR" in x, csvfile))
		# EXECUTION_TIME = list(filter(lambda x: "Total Time" in x, csvfile))
		#
		# error = max(list(map(lambda x : float(x.split(':')[1]), ABSOLUTE_ERROR)))
		#
		# print("\t",Message[conf], "->", "execution time :", EXECUTION_TIME[0])
		# print("\t",Message[conf], "->", "absolute error :", error)
		# fout.write("\t {message} -->  execution time = {exec_time}\n".format(message=Message[conf], exec_time=EXECUTION_TIME[0]))
		# fout.write("\t {message} -->  absolute error = {abs_err}\n\n\n".format(message=Message[conf], abs_err=error))
		pass


fout.close()
