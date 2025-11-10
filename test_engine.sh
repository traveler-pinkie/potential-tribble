#!/bin/bash
# Comprehensive test of engine features

echo "=== Engine Feature Tests ==="
echo ""

echo "Test 1: Engine with avgtime=0 (immediate moves)"
echo "Expected: Engine should make one move and exit"
echo -e "I7-G7\n" | timeout 5 ./bin/ccheck -w -d -a 0 2>&1 | grep -E "(@@@|Engine died)" | head -3
echo ""

echo "Test 2: Engine with avgtime=5 and verbose mode"
echo "Expected: Should see search progress output"
(sleep 1; echo "") | timeout 3 ./bin/ccheck -w -d -a 5 -v 2>&1 | grep -E "Searching depth|Nodes:" | head -5
echo ""

echo "Test 3: Tournament mode with @@@ prefix"
echo "Expected: All moves should have @@@ prefix"
echo -e "I7-G7\nI9-I7\n" | timeout 5 ./bin/ccheck -t -w -d -a 0 2>&1 | grep "@@@"
echo ""

echo "Test 4: Engine continues searching on opponent's time"
echo "Expected: Engine searches while waiting for opponent move"
(sleep 2; echo "I7-G7") | timeout 5 ./bin/ccheck -w -d -a 10 -v 2>&1 | grep -E "Searching depth" | head -10
echo ""

echo "Test 5: Both players using engine (two ccheck instances via pipes)"
echo "Expected: Multiple moves with engine calculating"
echo "Skipped: Requires complex pipe setup with versus utility"
echo ""

echo "=== All Engine Tests Complete ==="
