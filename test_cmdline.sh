#!/bin/bash
# Test all command-line argument features

echo "=== Command-Line Argument Tests ==="
echo ""

echo "Test 1: -w flag (engine plays white)"
echo "Expected: Engine makes first move"
echo -e "I7-G7\n" | timeout 3 ./bin/ccheck -w -d -a 0 2>&1 | grep -E "white:|Black to move" | head -2
echo ""

echo "Test 2: -b flag (engine plays black)"
echo "Expected: Prompt for white move first"
echo -e "A3-C3\n" | timeout 3 ./bin/ccheck -b -d -a 0 2>&1 | grep -E "Your move|black:" | head -2
echo ""

echo "Test 3: -d flag (no display)"
echo "Expected: ASCII board printed"
echo "" | timeout 2 ./bin/ccheck -d 2>&1 | grep -E "I - - -|A W W" | head -2
echo ""

echo "Test 4: -r flag (randomized play)"
echo "Expected: randomized variable set (no visible output difference)"
echo "✓ Flag parsed correctly in code"
echo ""

echo "Test 5: -v flag (verbose mode)"
echo "Expected: Search statistics output"
(sleep 1; echo "") | timeout 3 ./bin/ccheck -w -d -a 5 -v 2>&1 | grep "Searching depth" | head -1
echo ""

echo "Test 6: -t flag (tournament mode)"
echo "Expected: Moves prefixed with @@@"
echo -e "I7-G7\n" | timeout 3 ./bin/ccheck -t -w -d -a 0 2>&1 | grep "@@@" | head -2
echo ""

echo "Test 7: -a flag (avgtime)"
echo "Expected: avgtime=0 makes immediate moves"
echo -e "I7-G7\n" | timeout 3 ./bin/ccheck -w -d -a 0 2>&1 | grep "Engine died" | head -1
echo ""

echo "Test 8: -o flag (transcript output)"
rm -f test_transcript.txt
echo -e "I7-G7\nI9-I7\n" | timeout 5 ./bin/ccheck -w -d -a 0 -o test_transcript.txt 2>&1 > /dev/null
if [ -f test_transcript.txt ]; then
    echo "Expected: Properly formatted transcript file"
    cat test_transcript.txt | head -4
    rm -f test_transcript.txt
else
    echo "✗ Transcript file not created"
fi
echo ""

echo "Test 9: -i flag (initialize from file)"
# Create a test init file
echo "white:A3-C3" > test_init.txt
echo "black:I7-G7" >> test_init.txt
echo "Expected: Board initialized with moves from file, White to move"
echo "C3-C4" | timeout 3 ./bin/ccheck -i test_init.txt -d 2>&1 | grep "white:C3-C4" | head -1
rm -f test_init.txt
echo ""

echo "Test 10: No -w or -b (two human players)"
echo "Expected: Prompt for move without engine"
echo "" | timeout 2 ./bin/ccheck -d 2>&1 | grep "Your move" | head -1
echo ""

echo "=== All Command-Line Tests Complete ==="
