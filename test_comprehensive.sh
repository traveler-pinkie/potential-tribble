#!/bin/bash
# Comprehensive edge case testing

echo "=== EDGE CASE TESTING ==="
echo

PASS=0
FAIL=0

# Test 1: avgtime=0
echo -n "Test 1 - avgtime=0 with engine: "
OUTPUT=$(echo "quit" | timeout 3 ./bin/ccheck -d -w -a 0 2>&1)
if [ $? -eq 0 ] && echo "$OUTPUT" | grep -q "white:"; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 2: avgtime=1
echo -n "Test 2 - avgtime=1 with engine: "
OUTPUT=$(echo "quit" | timeout 5 ./bin/ccheck -d -b -a 1 2>&1)
if [ $? -eq 0 ]; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 3: Init file only (no engine)
echo -n "Test 3 - Init file without engine: "
OUTPUT=$(echo "quit" | timeout 3 ./bin/ccheck -d -i test_legal_moves.txt 2>&1)
if [ $? -eq 0 ]; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 4: Transcript output
echo -n "Test 4 - Transcript output: "
rm -f /tmp/transcript.txt
echo "quit" | timeout 5 ./bin/ccheck -d -w -a 0 -o /tmp/transcript.txt >/dev/null 2>&1
if [ -f /tmp/transcript.txt ] && grep -q "1\. white:" /tmp/transcript.txt; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 5: Tournament mode
echo -n "Test 5 - Tournament mode @@@ prefix: "
OUTPUT=$(echo "quit" | timeout 3 ./bin/ccheck -d -t -w -a 0 2>&1)
if echo "$OUTPUT" | grep -q "@@@white:"; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 6: Randomized flag
echo -n "Test 6 - Randomized flag (-r): "
OUTPUT=$(echo "quit" | timeout 3 ./bin/ccheck -d -w -r -a 0 2>&1)
if [ $? -eq 0 ] && echo "$OUTPUT" | grep -q "white:"; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 7: Init + transcript
echo -n "Test 7 - Init file with transcript: "
rm -f /tmp/transcript2.txt
echo "quit" | timeout 3 ./bin/ccheck -d -i test_legal_moves.txt -o /tmp/transcript2.txt 2>&1 >/dev/null
if [ -f /tmp/transcript2.txt ] && grep -q "black:I7-G7" /tmp/transcript2.txt; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 8: Verbose mode
echo -n "Test 8 - Verbose mode (-v): "
OUTPUT=$(echo "quit" | timeout 5 ./bin/ccheck -d -w -a 0 -v 2>&1)
if echo "$OUTPUT" | grep -q "Searching"; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 9: No flags (referee mode)
echo -n "Test 9 - Referee mode (no engine): "
OUTPUT=$(echo -e "white:A3-C3\nquit" | timeout 3 ./bin/ccheck -d 2>&1)
if [ $? -eq 0 ]; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

# Test 10: Memory check with valgrind
echo -n "Test 10 - Valgrind memory check: "
OUTPUT=$(echo "quit" | timeout 10 valgrind --leak-check=full --error-exitcode=1 ./bin/ccheck -d -w -a 0 2>&1)
if [ $? -eq 0 ]; then
    echo "PASS"
    PASS=$((PASS+1))
else
    echo "FAIL"
    FAIL=$((FAIL+1))
fi

echo
echo "=== RESULTS ==="
echo "Passed: $PASS/10"
echo "Failed: $FAIL/10"

if [ $FAIL -eq 0 ]; then
    echo "SUCCESS: All tests passed!"
    exit 0
else
    echo "FAILURE: Some tests failed"
    exit 1
fi
