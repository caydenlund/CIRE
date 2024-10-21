import os
import sys
import json


# This function summarises the json file results
def summarize(file_path):
    try:
        with open(file_path, 'r') as file:
            data = json.load(file)
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
    except json.JSONDecodeError:
        print(f"Error: Could not decode the JSON file '{file_path}'.")
    except Exception as e:
        print(f"An error occurred: {e}")

    # print(data)
    return data


# This function runs CIRE_LLVM on one LLVM file
def run_cire_llvm(llvm_file):
    # print(os.getcwd())
    # Run CIRE_LLVM on the LLVM file
    exit_code = os.system("../build-debug/bin/CIRE_LLVM -csv-friendly " + llvm_file)
    # Check if CIRE_LLVM ran successfully
    if exit_code != 0:
        print("CIRE_LLVM failed to run on " + llvm_file)
    else:
        print("CIRE_LLVM ran successfully on " + llvm_file)


# This function runs CIRE_LLVM on every .ll file in specified directory
def run_cire_llvm_on_dir(directory):
    # Get all the files in the directory
    files = os.listdir(directory)

    os.system("rm results.json default.log")

    # Iterate over all the files
    for file in files:
        # Check if the file is a .ll file
        if file.endswith(".ll"):
            # Run CIRE_LLVM on the file
            # print("Running CIRE_LLVM on " + directory + '/' + file)
            run_cire_llvm(directory + '/' + file)

    summarize("results.json")


# main function
def main():
    directory = sys.argv[1]
    # Run CIRE_LLVM on specified directory
    run_cire_llvm_on_dir(directory)


# Call the main function
if __name__ == "__main__":
    main()
