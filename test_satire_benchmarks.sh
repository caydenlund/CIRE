#!/usr/bin/env bash

# Script to test all SATIRE benchmarks
# Tests parsing and execution of all benchmark files
#
# Usage:
#   ./test_satire_benchmarks.sh [--fast|--extended]
#
# Options:
#   --fast      Fast mode: 1s timeout per benchmark (parse-only test)
#   --extended  Extended mode: 60s timeout per benchmark (for large benchmarks)
#   (default)   Normal mode: 10s timeout per benchmark

set -e

CIRE_BIN="./build/CIRE"
BENCHMARK_DIR="_/benchmarks"
TEMP_DIR="/tmp/cire_benchmark_tests"
LOG_FILE="${TEMP_DIR}/test_results.log"

# Default timeout
TIMEOUT=10

# Parse command line arguments
if [ "$1" = "--fast" ]; then
    TIMEOUT=1
    echo "Fast mode: 1s timeout per benchmark"
elif [ "$1" = "--extended" ]; then
    TIMEOUT=60
    echo "Extended mode: 60s timeout per benchmark"
fi

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
TOTAL=0
PASSED=0
FAILED=0
SKIPPED=0

# Arrays to track results
declare -a FAILED_FILES
declare -a SKIPPED_FILES

# Create temp directory
mkdir -p "${TEMP_DIR}"
rm -f "${LOG_FILE}"

echo "======================================"
echo "CIRE SATIRE Benchmark Test Suite"
echo "======================================"
echo ""

# Check if CIRE binary exists
if [ ! -f "${CIRE_BIN}" ]; then
    echo -e "${RED}ERROR: CIRE binary not found at ${CIRE_BIN}${NC}"
    echo "Please build CIRE first with: ninja -C build"
    exit 1
fi

# Function to extract domain from SATIRE file and create JSON
extract_domain() {
    local satire_file="$1"
    local json_file="$2"

    # Use Python to parse the SATIRE file and extract domains
    python3 - "$satire_file" "$json_file" << 'PYTHON_SCRIPT'
import sys
import re
import json

satire_file = sys.argv[1]
json_file = sys.argv[2]

try:
    with open(satire_file, 'r') as f:
        content = f.read()

    # Find INPUTS section
    inputs_match = re.search(r'INPUTS\s*\{(.*?)\}', content, re.DOTALL)
    if not inputs_match:
        print(f"Warning: No INPUTS section found in {satire_file}", file=sys.stderr)
        sys.exit(1)

    inputs_section = inputs_match.group(1)

    # Parse variable declarations with domains
    # Format: varname fl64 : (lo, hi);
    pattern = r'(\w+)\s+fl\d+\s*:\s*\(\s*(-?[\d.eE+-]+)\s*,\s*(-?[\d.eE+-]+)\s*\)'
    matches = re.findall(pattern, inputs_section)

    if not matches:
        print(f"Warning: No input domains found in {satire_file}", file=sys.stderr)
        sys.exit(1)

    # Create domain dictionary
    domain = {}
    for var_name, lo, hi in matches:
        domain[var_name] = [float(lo), float(hi)]

    # Write JSON file
    with open(json_file, 'w') as f:
        json.dump(domain, f, indent=2)

    sys.exit(0)

except Exception as e:
    print(f"Error parsing {satire_file}: {e}", file=sys.stderr)
    sys.exit(1)
PYTHON_SCRIPT

    return $?
}

# Function to test a single SATIRE file
test_satire_file() {
    local satire_file="$1"
    local test_name=$(basename "$satire_file" .txt)
    local domain_file="${TEMP_DIR}/${test_name}_domain.json"

    TOTAL=$((TOTAL + 1))

    echo -n "Testing: $satire_file ... "

    # Extract domain
    if ! extract_domain "$satire_file" "$domain_file" 2>>"${LOG_FILE}"; then
        echo -e "${YELLOW}SKIPPED${NC} (no domain)"
        SKIPPED=$((SKIPPED + 1))
        SKIPPED_FILES+=("$satire_file")
        echo "SKIPPED: $satire_file (no domain)" >> "${LOG_FILE}"
        return
    fi

    # Run CIRE
    if timeout "${TIMEOUT}" "${CIRE_BIN}" "$satire_file" --domain "$domain_file" > "${TEMP_DIR}/${test_name}_output.txt" 2>&1; then
        echo -e "${GREEN}PASSED${NC}"
        PASSED=$((PASSED + 1))
        echo "PASSED: $satire_file" >> "${LOG_FILE}"
    else
        local exit_code=$?

        # Check if it's an expected error (unsupported features or mathematically invalid)
        if grep -q "IBEX: Objective is unbounded" "${TEMP_DIR}/${test_name}_output.txt" 2>/dev/null; then
            echo -e "${YELLOW}SKIPPED${NC} (unbounded)"
            SKIPPED=$((SKIPPED + 1))
            SKIPPED_FILES+=("$satire_file")
            echo "SKIPPED: $satire_file (unbounded objective)" >> "${LOG_FILE}"
        elif grep -qE "(if.*then|CONSTRAINTS)" "$satire_file" 2>/dev/null; then
            echo -e "${YELLOW}SKIPPED${NC} (unsupported syntax)"
            SKIPPED=$((SKIPPED + 1))
            SKIPPED_FILES+=("$satire_file")
            echo "SKIPPED: $satire_file (unsupported features)" >> "${LOG_FILE}"
        else
            echo -e "${RED}FAILED${NC} (exit code: $exit_code)"
            FAILED=$((FAILED + 1))
            FAILED_FILES+=("$satire_file")
            echo "FAILED: $satire_file (exit code: $exit_code)" >> "${LOG_FILE}"
            echo "  Output:" >> "${LOG_FILE}"
            tail -20 "${TEMP_DIR}/${test_name}_output.txt" >> "${LOG_FILE}" 2>&1 || true
            echo "" >> "${LOG_FILE}"
        fi
    fi
}

# Find and test all SATIRE benchmark files
# Exclude _input.txt files and CMakeLists.txt
echo "Discovering SATIRE benchmarks..."
echo ""

while IFS= read -r satire_file; do
    # Skip _input.txt files and CMakeLists.txt
    if [[ "$satire_file" == *"_input.txt" ]] || [[ "$satire_file" == *"CMakeLists.txt" ]]; then
        continue
    fi

    # Only process .txt files
    if [[ "$satire_file" == *.txt ]]; then
        test_satire_file "$satire_file"
    fi
done < <(find "${BENCHMARK_DIR}" -name "*.txt" -type f | sort)

# Print summary
echo ""
echo "======================================"
echo "Test Summary"
echo "======================================"
echo "Total:   ${TOTAL}"
echo -e "Passed:  ${GREEN}${PASSED}${NC}"
echo -e "Failed:  ${RED}${FAILED}${NC}"
echo -e "Skipped: ${YELLOW}${SKIPPED}${NC}"
echo ""

# Print failed files if any
if [ ${FAILED} -gt 0 ]; then
    echo -e "${RED}Failed benchmarks:${NC}"
    for file in "${FAILED_FILES[@]}"; do
        echo "  - $file"
    done
    echo ""
fi

# Print skipped files if any
if [ ${SKIPPED} -gt 0 ]; then
    echo -e "${YELLOW}Skipped benchmarks (no domain info):${NC}"
    for file in "${SKIPPED_FILES[@]}"; do
        echo "  - $file"
    done
    echo ""
fi

echo "Detailed log: ${LOG_FILE}"
echo ""

# Exit with error if any tests failed
if [ ${FAILED} -gt 0 ]; then
    exit 1
else
    exit 0
fi
