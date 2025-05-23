# Write a program that outputs a C conjugate gradient program
import os
import sys
import copy

if __name__ == "__main__":
    print(os.getcwd())
    print("1: num iterations")
    N = int(sys.argv[1])

    dumpStr = ""

    # Compute initial residual
    R = [[0]*N for i in range(2)]
    P = [[0]*N for i in range(2)]
    for i in range(0, N):
        rhs_matvec_i = "b_{idx}".format(idx=i) + "-" + "+".join(["A_{idx}_{jdy}*x_{kid}_{jdy}".format(kid=str(k), idx=str(i), jdy=j) for j in range(N)])
        lhs_matvec_i = "r_{kid}_{idx}".format(kid=str(k), idx=i)
        R[0][i] = lhs_matvec_i ;
        dumpStr += "\t\tdouble " + lhs_matvec_i + " = " + rhs_matvec_i + ";\n"
    P = copy.deepcopy(R)

    while(k < L):
        ## alpha = r^T r
        rhs_RTR = "+".join(["{Ri}*{Ri}".format(Ri = R[0][i]) for i in range(N)])
        lhs_RTR = "rtr_{kid}".format(kid=k)
        dumpStr += "\t\tdouble " + lhs_RTR + " = " + rhs_RTR + ";\n"

        rhs_AP = [0]*N

        for i in range(0, N):
            rhs_AP_i = "+".join(["A_{idx}_{jdy} * {Pi}".format(idx = str(i), jdy = str(j), Pi = P[0][i]) for j in range(N)])
            lhs_AP_i = "AP_{kid}_{idx}".format(kid=str(k), idx=i)

            rhs_AP[i] = lhs_AP_i
            dumpStr += "\t\tdouble " + lhs_AP_i + " = " + rhs_AP_i + ";\n"

        rhs_PAP = "+".join(["{Pi}*{APi}".format(Pi = P[0][i], APi = rhs_AP[i]) for i in range(N)])
        lhs_PAP = "pap_{kid}".format(kid=k)
        dumpStr += "\t\tdouble " + lhs_PAP + " = " + rhs_PAP + ";\n"

        rhs_alpha = "({numer})/({denom})".format(numer = lhs_RTR, denom = lhs_PAP)
        lhs_alpha = "alpha_{kid}".format(kid = k)
        dumpStr += "\t\tdouble " + lhs_alpha + " = " + rhs_alpha + ";\n"

        for i in range(N):
            rhs_x = "x_{kid}_{idx} + {alpha}*{Rid}".format(alpha = lhs_alpha, kid = k, idx = i, Rid = R[0][i])
            lhs_x = "x_{kpid}_{idx}".format(kpid = str(k+1), idx = str(i))
            dumpStr += "\t\tdouble " + lhs_x + " = " + rhs_x + ";\n"

            ## update next residue r1

        for i in range(0, N):
            rhs_next_residue = "{Rid} - {alpha}*{APi}".format(Rid=R[0][i], alpha=lhs_alpha, APi = rhs_AP[i])
            lhs_next_residue = "r_{kpid}_{idx}".format(kpid = k+1, idx= i)
            R[1][i] = lhs_next_residue

            dumpStr += "\t\tdouble " + lhs_next_residue + " = " + rhs_next_residue + ";\n"

        rhs_r1tr1 = "+".join(["{R1id}*{R1id}".format(R1id = R[1][i]) for i in range(N)])
        lhs_r1tr1 = "r1tr1_{kid}".format(kid=k)
        dumpStr += "\t\tdouble " + lhs_r1tr1 + " = " + rhs_r1tr1 + ";\n"
        rhs_r0tr0 = "+".join(["{R0id}*{R0id}".format(R0id = R[0][i]) for i in range(N)])
        lhs_r0tr0 = "r0tr0_{kid}".format(kid=k)
        dumpStr += "\t\tdouble " + lhs_r0tr0 + " = " + rhs_r0tr0 + ";\n"

        rhs_beta = "({numer})/({denom})".format(numer=lhs_r1tr1, denom=lhs_r0tr0)
        lhs_beta = "beta_{kid}".format(kid=k)
        dumpStr += "\t\tdouble " + lhs_beta + " = " + rhs_beta + ";\n"

        for i in range(0, N):
            rhs_next_p = "{R1id} + {beta}*{p0id}".format(R1id = R[1][i], beta=lhs_beta, p0id=P[0][i])
            lhs_next_p = "p_{kpid}_{idx}".format(kpid = k+1, idx=i)
            P[1][i] = lhs_next_p
            dumpStr += "\t\tdouble " + lhs_next_p + " = " + rhs_next_p + ";\n"

        P[0] = P[1]
        R[0] = R[1]

        k += 1

    outfile = "CG_"+tagname+"_K"+str(k)+"_N"+str(N)+".cpp"
    fout = open("scratch/"+outfile, 'w')

    fout.write("#include <iostream>\n")
    fout.write("#include <cmath>\n\n")

    fout.write("using namespace std;\n\n")

    ## Print the inputs ##
    fout.write("double src(\n")
    for i in range(N):
        fout.write("\t")
        for j in range(N):
            fout.write("double A_{idx}_{jdy},".format(idx=i, jdy=j))
        fout.write("\n")
    fout.write("\t")
    for i in range(N):
        fout.write("double b_{idx},".format(idx=i))
    fout.write("\n")
    fout.write("\t")
    for i in range(N-1):
        fout.write("double x_0_{idx},".format(idx=i))
    fout.write("double x_0_{idx}".format(idx=N-1))
    fout.write(") {\n")

    ## print the matvec operation between A and x
    fout.write(dumpStr)
    fout.write("\t\treturn x_"+str(k)+"_"+str(N-1)+";\n")
    fout.write("}\n")

    # bval = str(b[i])
    # val = A[i][j]
    # if val == 0:
    #     fout.write("\t"+var+"\t fl64 : ("+str(val)+" , "+str(val)+");\n")
    # else:
    #     fout.write("\t"+var+"\t fl64 : ("+str(val-err)+" , "+str(val+err)+");\n")

    # write main function calling the src function
    fout.write("int main() {\n")
    fout.write("\t// Initialize the input values\n")
    for i in range(N):
        fout.write("\tdouble b_"+str(i)+" = "+str(b[i])+";\n")
        for j in range(N):
            fout.write("\tdouble A_"+str(i)+"_"+str(j)+" = "+str(A[i][j])+";\n")
    for i in range(N):
        fout.write("\tdouble x_0_"+str(i)+" = 0.0;\n")
    fout.write("\n")
    fout.write("\t// Call the src function\n")
    fout.write("\tdouble result = src(\n")
    for i in range(N):
        for j in range(N):
            fout.write("\t\tA_"+str(i)+"_"+str(j)+", ")
        fout.write("\n")
    for i in range(N):
        fout.write("\t\tb_"+str(i)+", ")
    fout.write("\n")
    for i in range(N-1):
        fout.write("\t\tx_0_"+str(i)+", ")
    fout.write("\tx_0_"+str(N-1)+");\n")
    fout.write("\n")
    fout.write("\t// Print the result\n")
    fout.write("\tcout << result << endl;\n")
    fout.write("\n")
    fout.write("\treturn 0;\n")
    fout.write("}\n")


    fout.close()

    print(dumpStr)