#!/bin/bash
# Example: Using CIRE Docker container
# This script demonstrates how to use CIRE in a Docker container

set -e

echo "========================================="
echo "CIRE Docker Container Example"
echo "========================================="
echo ""

# Create a temporary directory
TMPDIR=$(mktemp -d)
cd "$TMPDIR"

echo "Working in: $TMPDIR"
echo ""

# Create example C code
cat > cube.c << 'EOF'
// Compute x^3
double cube(double x) {
    return x * x * x;
}
EOF

echo "Created cube.c:"
cat cube.c
echo ""

# Create domain file
cat > domain.json << 'EOF'
{
  "x": [-1000000, 1000000]
}
EOF

echo "Created domain.json:"
cat domain.json
echo ""

# Run CIRE in Docker
echo "========================================="
echo "Running CIRE analysis in Docker..."
echo "========================================="
echo ""

docker run --rm -v "$TMPDIR:/workspace" cire:latest bash -c "
  echo 'Compiling cube.c to LLVM IR...'
  clang -S -emit-llvm -O0 cube.c -o cube.ll
  echo ''
  echo 'Generated LLVM IR:'
  cat cube.ll
  echo ''
  echo '========================================='
  echo 'Running CIRE error analysis...'
  echo '========================================='
  echo ''
  CIRE_LLVM cube.ll --domain domain.json --function cube
"

echo ""
echo "========================================="
echo "Example complete!"
echo "========================================="
echo ""
echo "Output files in: $TMPDIR"
echo ""
echo "To clean up: rm -rf $TMPDIR"
