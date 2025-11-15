# Versus Driver Compatibility Verification

## Overview
This document verifies that the `ccheck` implementation meets all requirements for compatibility with the `versus` driver program.

## Requirements from README(5).md

### 1. Tournament Mode (-t flag) ✅
**Requirement**: Program must accept `-t` flag to enable tournament mode.

**Implementation**: 
- `ccheck.c` line 104: Parses `-t` flag and sets `tournament_mode` variable
- When `tournament_mode` is set, display is not used for input (line 402)

**Status**: ✅ IMPLEMENTED

---

### 2. Standard Input/Output Communication ✅
**Requirement**: In tournament mode, accept moves via stdin and output moves via stdout.

**Implementation**:
- Lines 416-426: When not engine's turn and in tournament mode, reads from stdin using `read_move_interactive()`
- Lines 435-439: All moves are printed to stdout using `print_move()`

**Status**: ✅ IMPLEMENTED

---

### 3. Computer Moves Prefixed with @@@ ✅
**Requirement**: "Moves made by the computer in tournament mode are printed to the standard output... each move is prefixed by the sequence `@@@`"

**Implementation**: 
- Lines 435-437:
```c
if (tournament_mode && current_player == engine_player) {
    printf("@@@");
}
```
Only computer-generated moves get the @@@ prefix.

**Status**: ✅ IMPLEMENTED

---

### 4. Interactive Moves NOT Prefixed ✅
**Requirement**: Opponent moves (received via stdin) should NOT have @@@ prefix when echoed.

**Implementation**:
- Lines 435-437: Condition checks `current_player == engine_player`
- When opponent moves (read from stdin), `current_player != engine_player`, so no @@@ prefix

**Status**: ✅ IMPLEMENTED

---

### 5. Display Optional in Tournament Mode ✅
**Requirement**: "use the graphical display (if selected) only to show the current board position"

**Implementation**:
- Line 402: `!no_display && !tournament_mode` prevents display from being used for input in tournament mode
- Lines 441-452: Display is still updated if enabled, but not used for input

**Status**: ✅ IMPLEMENTED

---

### 6. Engine Can Play White or Black ✅
**Requirement**: Both `-w` and `-b` flags should work with `-t` flag.

**Implementation**:
- Lines 86-90: Parses `-w` and `-b` flags to set `engine_player`
- Engine process creation (lines 289-354) works for either player
- Move generation (lines 386-396) works when `current_player == engine_player`

**Status**: ✅ IMPLEMENTED

---

### 7. Time Control with -a Flag ✅
**Requirement**: Average time per move should be configurable.

**Implementation**:
- Line 107: Parses `-a` flag and sets global `avgtime` variable
- `engine.c` uses `avgtime` to control search depth and time limits

**Status**: ✅ IMPLEMENTED

---

### 8. Proper Process Management ✅
**Requirement**: Clean startup, shutdown, and signal handling.

**Implementation**:
- Lines 125-157: Signal handlers for SIGINT, SIGHUP, SIGPIPE, SIGCHLD, SIGTERM
- Lines 509-517: Cleanup processes on exit
- Lines 47-56: `cleanup_processes()` kills child processes with SIGKILL

**Status**: ✅ IMPLEMENTED

---

## Versus Driver Usage Pattern

### Expected Command Line
```bash
# White player (first to move)
util/versus -w bin/ccheck -t -w -a 10

# Black player (second to move)
util/versus -b <hostname> <port> bin/ccheck -t -b -a 10
```

### Communication Flow

1. **Versus Driver to ccheck**: 
   - Sends opponent moves via stdin in format: `player:from-to`
   - Example: `white:A3-C3` or `black:I7-G7`

2. **ccheck to Versus Driver**:
   - Outputs own moves via stdout with @@@ prefix
   - Example: `@@@white:A3-C3` or `@@@black:I7-G7`

3. **Versus Driver Processing**:
   - Strips @@@ from output moves
   - Forwards stripped moves to opponent as stdin input
   - Filters out non-move output (prompts, board displays, etc.)

### Data Flow Diagram
```
Program A (White)                versus                Program B (Black)
─────────────────               ───────               ─────────────────
                                   │
Engine generates move              │
@@@white:A3-C3 ─────────────────> │
                                   │ strips @@@
                                   │ white:A3-C3 ──────────> stdin
                                   │                         reads move
                                   │                         Engine responds
                            stdin <──────── @@@black:I7-G7
reads move                         │ strips @@@
Engine responds                    │
@@@white:C3-C4 ─────────────────> │
                                   ...
```

## Test Results

### Automated Tests
Run `test_versus_automated.sh` to verify:
- ✅ Tournament mode flag accepted
- ✅ Engine moves have @@@ prefix
- ✅ Move exchange works correctly
- ✅ Both white and black engines work
- ✅ avgtime=0 causes immediate moves
- ✅ Correct move format (player:from-to)
- ✅ Accepts opponent moves from stdin

### Manual Testing
See `test_versus_manual.md` for step-by-step manual testing with actual versus driver.

## Implementation Details

### Key Code Sections

**Tournament Mode Detection** (ccheck.c:402-416):
```c
} else if (!no_display && !tournament_mode && display_in && display_out) {
    // Interactive mode with display - request move from display
    ...
} else {
    // Interactive mode from stdin (used in tournament mode)
    move = read_move_interactive(board);
    ...
}
```

**@@@ Prefix Logic** (ccheck.c:435-439):
```c
// Print move - in tournament mode, prefix computer moves with @@@
if (tournament_mode && current_player == engine_player) {
    printf("@@@");
}
print_move(board, move, stdout);
printf("\n");
fflush(stdout);
```

**Engine Communication** (engine.c:172-207):
```c
if (line[0] == '<') {
    // Request to generate a move
    ...
    print_move(board, best, stdout);
    printf("\n");
    fflush(stdout);
} else if (line[0] == '>') {
    // Opponent's move received
    ...
}
```

## Compatibility Checklist

- [x] Accepts `-t` flag for tournament mode
- [x] Reads opponent moves from stdin
- [x] Outputs own moves to stdout
- [x] Prefixes computer moves with @@@
- [x] Does NOT prefix opponent moves with @@@
- [x] Display optional (use -d to disable)
- [x] Engine can play White (-w)
- [x] Engine can play Black (-b)
- [x] Time control with -a <seconds>
- [x] avgtime=0 for immediate moves
- [x] Proper signal handling (SIGINT, SIGTERM, etc.)
- [x] Clean process management
- [x] Graceful shutdown on errors
- [x] Move format: `player:from-to` or `player:from-via-to`

## Conclusion

✅ **The implementation is fully compatible with the versus driver.**

All required features are implemented correctly:
- Tournament mode operation
- Correct move formatting with @@@ prefix
- Proper stdin/stdout communication
- Engine process management
- Signal handling and cleanup

The program is ready for competitive play using the versus driver.
