# CSE 320 HW4 Implementation Summary

## Completed Features

### 1. Tournament Mode (`-t` flag)
**File**: `src/ccheck.c`

**Implementation**:
- Parses `-t` flag in command-line arguments
- In tournament mode, ALL moves (whether from engine or stdin) are printed with `@@@` prefix
- Allows moves from either engine or standard input
- Display (if used) only shows board position, not used for input
- Compatible with `versus` utility for network play

**Key Code Changes**:
```c
if (tournament_mode) {
    printf("@@@");
    print_move(board, move, stdout);
    printf("\n");
    fflush(stdout);
}
```

### 2. Engine Process Implementation
**File**: `src/engine.c`

**Core Features**:

#### a. Iterative Deepening Search
- Searches game tree from depth 1 to MAXPLY (10 plies)
- Each iteration deepens the search by one ply
- Continues until time limit, SIGHUP received, or max depth reached

#### b. Time Control
- **avgtime = 0**: Only searches depth 1, makes immediate moves
- **avgtime > 0**: Uses sophisticated time budget calculation:
  ```
  time_budget = avgtime * moves_we_made
  time_available = avgtime + (time_budget - total_time_used)
  ```
- Avoids starting a search if `times[depth] > time_available`
- Sets SIGALRM timer to enforce time limits

#### c. Searching on Opponent's Time
- When `our_turn == 0`, engine continues searching with no time limit
- Searches deeper and deeper until SIGHUP received
- Allows engine to "think ahead" during opponent's turn

#### d. Signal Handling
- **SIGHUP**: Interrupts search, indicates it's our turn to move
- **SIGALRM**: Time limit reached, stops current search depth
- Uses `sigsetjmp`/`siglongjmp` for non-local jumps from deep in search tree
- `in_search` flag prevents race conditions during signal handling

#### e. Principal Variation Management
- Tracks best move sequence found at each depth
- When making our move: shifts `principal_var` array down, decrements `depth_completed`
- When opponent moves: checks if it matches prediction
  - If match: shift array (predicted line still valid)
  - If no match: reset `depth_completed = 0` (need to re-search)

#### f. Communication Protocol
- Reads commands from stdin:
  - `<\n`: Generate and output our move
  - `>move\n`: Apply opponent's move to internal board
- Outputs moves to stdout
- Announces "Engine ready\n" for synchronization with main process

## Testing

### Test Scripts Created
1. **test_tournament.sh**: Verifies `@@@` prefix in tournament mode
2. **test_engine.sh**: Comprehensive engine feature tests
   - avgtime=0 (immediate moves)
   - Iterative deepening
   - Searching on opponent's time
   - SIGHUP interruption
   - Verbose output

### Test Results
All tests pass successfully:
- ✅ Tournament mode with @@@ prefix
- ✅ Engine makes immediate moves with avgtime=0
- ✅ Iterative deepening (depths 1-8+ observed)
- ✅ Search continues on opponent's time
- ✅ SIGHUP properly interrupts search
- ✅ Time control prevents exceeding avgtime budget
- ✅ Verbose mode shows search statistics

## Build and Run

```bash
# Build
make clean && make

# Tournament mode - engine plays white
./bin/ccheck -t -w -d -a 10

# Engine plays white with display
./bin/ccheck -w -a 10

# Verbose mode - see search progress
./bin/ccheck -w -a 5 -v

# Immediate moves (no time limit)
./bin/ccheck -w -d -a 0

# Network play with versus
util/versus -w ./bin/ccheck -t -w -a 10
util/versus -b <host> <port> ./bin/ccheck -t -b -a 10
```

## Key Implementation Details

### Time Control Calculation
The engine tracks:
- `avgtime`: Target average seconds per move
- `xtime`, `otime`: Total time used by each player
- `times[depth]`: Estimated search time for each depth
- `moves_made`: Number of moves in game so far

For each depth, checks if estimated search time fits in available budget before starting.

### Signal Safety
- All signal handlers only set `sig_atomic_t` flags
- Main loop checks flags between operations
- `siglongjmp` used only when `in_search` flag is set
- Prevents race conditions and signal handler reentrancy issues

### Process Architecture
- Main process: Game coordinator, handles I/O, moves
- Display process (optional): Graphical board via `xdisp`
- Engine process (optional): Computer player via `engine()`

All processes communicate via pipes with SIGHUP synchronization protocol.

## Files Modified
- `src/ccheck.c`: Main game logic, tournament mode, display/engine forking
- `src/engine.c`: Complete engine implementation with time control
- `util/versus`, `util/xdisp`: Made executable

## Git Repository
- Repository: https://github.com/traveler-pinkie/potential-tribble
- Branch: main
- Latest commit: "Complete HW4: Implement tournament mode and fix engine time control"
- Status: ✅ Synced with GitHub
