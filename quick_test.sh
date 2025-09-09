#!/bin/bash

# Quick test script for xdot_cpp library

echo "=== Quick xdot_cpp Test ==="

cd "$(dirname "$0")"

# Test files
tests=(
    "examples/simple.dot:Basic graph"
    "examples/complex.dot:Complex with subgraphs"
    "tests/large_graph.dot:Large graph"
    "tests/shapes_test.dot:Various shapes"
    "tests/nested_subgraphs.dot:Nested subgraphs"
    "tests/edge_cases.dot:Edge cases"
    "tests/undirected_graph.dot:Undirected graph"
    "tests/unicode_test.dot:Unicode test (may have issues)"
)

for test in "${tests[@]}"; do
    file="${test%%:*}"
    desc="${test##*:}"
    
    echo -n "Testing $file ($desc)... "
    
    if timeout 3s ./build/xdot_viewer "$file" >/dev/null 2>&1; then
        echo "✓"
    else
        echo "❌ (timeout or error)"
    fi
done

echo
echo "Note: Unicode test may show parsing errors due to special characters."
echo "This is a known limitation that could be improved in future versions."