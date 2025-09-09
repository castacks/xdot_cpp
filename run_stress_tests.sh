#!/bin/bash

# Fixed stress test script for xdot_cpp library
# Tests various DOT files to ensure robustness

echo "=== xdot_cpp Stress Test Suite ==="
echo "Testing various DOT file formats and edge cases..."
echo

# Build the project first
echo "Building xdot_cpp..."
cd "$(dirname "$0")"
if make -C build > /dev/null 2>&1; then
    echo "✓ Build successful"
else
    echo "❌ Build failed"
    exit 1
fi
echo

# Test files to run
TEST_FILES=(
    "examples/simple.dot:Basic directed graph"
    "examples/complex.dot:Complex graph with subgraphs"
    "tests/large_graph.dot:Large graph with many nodes and edges"
    "tests/shapes_test.dot:Various node shapes and edge styles"
    "tests/nested_subgraphs.dot:Deeply nested subgraph structures"
    "tests/unicode_test.dot:Unicode characters and special symbols"
    "tests/edge_cases.dot:Edge cases and unusual syntax"
    "tests/undirected_graph.dot:Undirected graph (graph vs digraph)"
)

PASSED=0
FAILED=0
TIMEOUT_DURATION=3

echo "Running tests..."
echo "=================="

for test_entry in "${TEST_FILES[@]}"; do
    test_file="${test_entry%%:*}"
    test_desc="${test_entry##*:}"
    
    if [ ! -f "$test_file" ]; then
        echo "❌ SKIP: $test_file (file not found)"
        continue
    fi
    
    echo -n "Testing $test_file ($test_desc)... "
    
    # Create a temporary log file
    log_file=$(mktemp)
    
    # Run the test with timeout in background
    ./build/xdot_viewer "$test_file" > "$log_file" 2>&1 &
    test_pid=$!
    
    # Wait for startup
    sleep 2
    
    # Check if process is still running and kill it
    if ps -p $test_pid > /dev/null 2>&1; then
        kill $test_pid 2>/dev/null
        wait $test_pid 2>/dev/null
        
        # Check for success indicators in log
        if grep -q "Window shown" "$log_file"; then
            echo "✓ PASS"
            PASSED=$((PASSED + 1))
        elif grep -q "Parsed DOT structure successfully" "$log_file" && ! grep -q "Error\|Failed" "$log_file"; then
            echo "✓ PASS (parsed successfully)"
            PASSED=$((PASSED + 1))
        else
            echo "❌ FAIL"
            echo "   Log excerpt:"
            tail -2 "$log_file" | sed 's/^/   /'
            FAILED=$((FAILED + 1))
        fi
    else
        echo "❌ FAIL (process died early)"
        echo "   Log excerpt:"
        tail -2 "$log_file" | sed 's/^/   /'
        FAILED=$((FAILED + 1))
    fi
    
    # Clean up
    rm -f "$log_file"
done

echo
echo "=================="
echo "Test Results:"
echo "  Passed: $PASSED"
echo "  Failed: $FAILED"
echo "  Total:  $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo
    echo "🎉 All tests passed! xdot_cpp is working correctly."
    exit 0
else
    echo
    echo "⚠️  Some tests failed. Please check the output above."
    exit 1
fi