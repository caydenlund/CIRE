# SATIRE Benchmark Testing

This document describes how to run the SATIRE benchmark test suite.

## Test Script

The `test_satire_benchmarks.sh` script automatically:
1. Discovers all SATIRE benchmark files in `_/benchmarks`
2. Extracts input domain specifications from each file
3. Runs CIRE on each benchmark
4. Reports pass/fail status with detailed logs

## Usage

### Quick Test
Run with default 10-second timeout per benchmark:
```bash
./test_satire_benchmarks.sh
```

### Fast Parse-Only Test
Test only that files parse correctly (1-second timeout):
```bash
./test_satire_benchmarks.sh --fast
```

### Extended Test
Run with longer timeout for large benchmarks (60-second timeout):
```bash
./test_satire_benchmarks.sh --extended
```

## Output

The script produces:
- **Console output**: Real-time progress with color-coded results
- **Log file**: Detailed results in `/tmp/cire_benchmark_tests/test_results.log`
- **Summary**: Total, passed, failed, and skipped counts

## Test Results

### Status Codes
- **PASSED** (green): Benchmark parsed and analyzed successfully
- **FAILED** (red): Error during parsing or analysis
- **SKIPPED** (yellow): No domain information found in file

### Common Failure Reasons
- **Exit code 124**: Timeout (benchmark too large or optimization slow)
- **Exit code 134**: Runtime error (unbounded objective, parse error, etc.)
- **Parse errors**: Unsupported SATIRE syntax
- **Unbounded objective**: Division by zero or unbounded intervals

## Current Test Status

### Passing Benchmarks (9)
- `abstraction_testing/test1/test1.txt`
- `binary_ops/addition/addition.txt`
- `binary_ops/multiplication/multiplication.txt`
- `binary_ops/subtraction/subtraction.txt`
- `composition/variable_literal/add_sub/add_sub.txt`
- `example2/example2.txt`
- `medium_benchmarks/lor_f64.txt`
- `satire_abs_20_25/dqmom/dqmom.txt`
- `satire_abs_20_25/horner/f_horner_50.txt`

### Known Issues
1. **Division benchmark**: Unbounded objective (likely division by interval containing zero)
2. **Large benchmarks**: Timeout with 10s limit (need extended mode)
3. **Parser errors**: Some advanced SATIRE syntax not yet supported

## Requirements

- CIRE must be built: `ninja -C build`
- Python 3 for domain extraction
- Benchmark directory `_/benchmarks` must exist

## Troubleshooting

### All tests skip
- Check that `_/benchmarks` directory exists
- Verify Python 3 is installed

### High timeout rate
- Use `--extended` mode for large benchmarks
- Some benchmarks may take minutes to analyze

### Parse errors
- Check SATIRE file format matches supported syntax
- See parser implementation in `src/frontend/satire/`
