#!/bin/bash
# Test for race conditions and flaky behavior

echo "Testing for race conditions with multiple iterations..."
FAILED=0

for i in {1..20}; do
    echo -n "Iteration $i: "
    OUTPUT=$(echo -e "white:A4-C4\nquit" | timeout 5 ./bin/ccheck -d -b -a 0 -i test_legal_moves.txt 2>&1)
    EXIT_CODE=$?
    
    if [ $EXIT_CODE -eq 124 ]; then
        echo "TIMEOUT"
        FAILED=$((FAILED+1))
    elif [ $EXIT_CODE -ne 0 ]; then
        echo "FAILED (exit code $EXIT_CODE)"
        FAILED=$((FAILED+1))
    elif echo "$OUTPUT" | grep -q "black:"; then
        echo "OK"
    else
        echo "NO ENGINE OUTPUT"
        FAILED=$((FAILED+1))
    fi
done

echo ""
echo "Results: $((20-FAILED))/20 passed"

if [ $FAILED -eq 0 ]; then
    echo "SUCCESS: No race conditions detected"
    exit 0
else
    echo "FAILURE: $FAILED iterations failed"
    exit 1
fi
