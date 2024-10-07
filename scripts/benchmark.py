import os
import sys


# This function runs CIRE_LLVM on every .ll file in specified directory
def run_cire_llvm_on_dir(directory):
    # Get all the files in the directory
    files = os.listdir(directory)

    # Iterate over all the files
    for file in files:
        # Check if the file is a .ll file
        if file.endswith(".ll"):
            # Run CIRE_LLVM on the file
            print("Running CIRE_LLVM on " + directory + file)
            run_cire_llvm(directory + file)


# This function runs CIRE_LLVM on one LLVM file
def run_cire_llvm(llvm_file):
    # Run CIRE_LLVM on the LLVM file
    exit_code = os.system("../build-debug/bin/CIRE_LLVM " + llvm_file)
    # Check if CIRE_LLVM ran successfully
    if exit_code != 0:
        print("CIRE_LLVM failed to run on " + llvm_file)
    else:
        print("CIRE_LLVM ran successfully on " + llvm_file)


# main function
def main():
    directory = sys.argv[1]
    # Run CIRE_LLVM on specified directory
    run_cire_llvm_on_dir(directory)


# Call the main function
if __name__ == "__main__":
    main()
