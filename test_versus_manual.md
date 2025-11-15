# Manual Versus Driver Test

## Quick Functionality Check

### Test 1: Tournament Mode Output Format

```bash
# Test that computer moves have @@@ prefix
echo "I7-G7" | ./bin/ccheck -t -w -d -a 0
```

Expected output should include:
```
Your move? @@@white:A3-C3
```
or similar, showing the @@@ prefix on the computer's move.

### Test 2: Interactive Move (No @@@ prefix)

When in tournament mode with the engine playing Black:
```bash
echo -e "A3-C3\nquit" | ./bin/ccheck -t -b -d -a 0
```

The move "white:A3-C3" should be printed WITHOUT @@@ because it's an interactive move (opponent's move that we're just echoing/confirming).

Actually, looking at the code and README more carefully: ALL moves are printed to stdout (as confirmation), but only computer-generated moves get the @@@ prefix.

### Test 3: Full Versus Driver Test (Two Terminals Required)

**Terminal 1:**
```bash
util/versus -w bin/ccheck -t -w -a 5
```

Wait for output showing hostname and port.

**Terminal 2:** (using info from Terminal 1)
```bash
util/versus -b <hostname> <port> bin/ccheck -t -b -a 5
```

Watch the game play out. Both programs should:
- Accept moves via stdin (from versus)
- Output their own moves with @@@ prefix
- Continue until game ends or interrupted

### Test 4: Same Computer Test

You can run both on the same machine using localhost:

**Terminal 1:**
```bash
util/versus -w bin/ccheck -t -w -a 2
```

Note the port number (e.g., 49963)

**Terminal 2:**
```bash
util/versus -b localhost 49963 bin/ccheck -t -b -a 2
```

### Expected Behavior

✓ Program accepts opponent moves from stdin
✓ Program outputs own moves with @@@ prefix
✓ Program continues alternating moves
✓ Display (if enabled) shows current position but doesn't input moves
✓ Game ends when one player wins
✓ Clean shutdown on SIGINT (Ctrl+C)

### Common Issues

1. **Timeout/Hang**: Engine might be taking too long. Use `-a 0` for immediate moves.
2. **No @@@ prefix**: Check that tournament mode (-t) is enabled.
3. **Wrong prefix on moves**: Verify only engine moves get @@@, not interactive.
4. **Engine dies**: Check that engine process starts correctly.
5. **Connection refused**: Make sure first versus instance is running before starting second.

## Implementation Requirements Met

- [x] Tournament mode (-t flag)
- [x] Computer moves prefixed with @@@
- [x] Interactive moves NOT prefixed with @@@
- [x] Standard input/output for communication
- [x] Display optional (use -d to disable)
- [x] Engine can play White (-w) or Black (-b)
- [x] Time control with -a flag
- [x] Clean process management and signal handling
