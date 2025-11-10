#!/bin/bash
# Test tournament mode functionality

echo "Testing tournament mode with engine playing white..."
echo ""

# Test 1: Engine plays white, reads black moves from input
echo "Test 1: Engine as White, reading Black moves from stdin"
echo "Expected: All moves should have @@@ prefix"
echo ""

(sleep 1; echo "I7-G7"; sleep 1; echo "I9-I7") | timeout 10 ./bin/ccheck -t -w -d -a 0 2>&1 | grep -E "(@@@|Your move)" | head -10

echo ""
echo "Test 2: Engine plays black, reads white moves from stdin"
echo "Expected: All moves should have @@@ prefix"
echo ""

(sleep 1; echo "A3-C3"; sleep 1; echo "A1-A3") | timeout 10 ./bin/ccheck -t -b -d -a 0 2>&1 | grep -E "(@@@|Your move)" | head -10

echo ""
echo "Tournament mode tests complete!"
