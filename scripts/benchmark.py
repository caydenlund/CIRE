import os
import sys
import glob
import json
import shutil
import functools

# Do not assign any value to this variable. Script will find the path to clang.
CLANG_PATH = ""


# Finds clang in the specified directory or in the PATH
def find_clang(directory):
    """Finds the path to clang in the specified directory."""

    if directory:
        for root, _, files in os.walk(directory):
            for file in files:
                if file == "clang":
                    return os.path.join(root, file)

    # If clang is not found in the specified directory, search the PATH
    if shutil.which("clang"):
        return shutil.which("clang")
    else:
        return "clang not found"
        exit(0)


# This function runs clang on every .c/.cpp file in the specified directory and outputs LLVM IR
# to another directory in files with the same name but with .ll extension
def run_clang_on_dir(directory, flag_list):
    # Get all the files in the directory
    files = os.listdir(directory)
    directory_extension = functools.reduce(lambda x, y: x + "_" + y, flag_list, "")
    destination_dir = directory + directory_extension

    flags = functools.reduce(lambda x, y: x + " -" + y, flag_list, "")

    # Iterate over all the files
    for file in files:
        # Check if the file is a .c or .cpp file
        if file.endswith(".c"):
            # Run clang on the file
            print("Running clang on " + directory + '/' + file)
            os.system(CLANG_PATH + "clang" + flags + " -S -emit-llvm " + directory + '/' + file + " -o " +
                      destination_dir + '/' + file[:-2] + ".ll")
        elif file.endswith(".cpp"):
            # Run clang++ on the file
            print("Running clang++ on " + directory + '/' + file)
            os.system(CLANG_PATH + "clang++" + flags + " -S -emit-llvm " + directory + '/' + file + " -o " +
                      destination_dir + '/' + file[:-4] + ".ll")


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
    files = glob.glob(directory + '/*.ll')

    os.system("rm results.json default.log")

    # Iterate over all the files
    for file in files:
        # Run CIRE_LLVM on the file
        # print("Running CIRE_LLVM on " + directory + '/' + file)
        run_cire_llvm(file)

    summarize("results.json")


# main function
def main():
    # Provide path to the directory containing the .c/.cpp files
    directory = sys.argv[1]
    clang_search_path = ""
    # Provide directory containing clang if it is not in the PATH
    if len(sys.argv) > 2:
        clang_search_path = sys.argv[2]

    # Provide flags to pass to clang
    flag_list = ["O3", "ffast-math"]

    global CLANG_PATH
    CLANG_PATH = os.path.dirname(find_clang(clang_search_path))
    print("Clang found at: " + CLANG_PATH)

    # run_clang_on_dir(directory, flag_list)

    # directory_extension = functools.reduce(lambda x, y: x + "_" + y, flag_list, "")
    # destination_dir = directory + directory_extension

    # Run CIRE_LLVM on specified directory
    run_cire_llvm_on_dir(directory)


# Call the main function
if __name__ == "__main__":
    main()
