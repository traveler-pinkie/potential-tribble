/*
 * Chinese Checkers Engine
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <sys/time.h>

#include "ccheck.h"
#include "debug.h"

static volatile sig_atomic_t sighup_received = 0;
static volatile sig_atomic_t time_to_move = 0;
static volatile sig_atomic_t in_search = 0;
static sigjmp_buf env;

// Signal handler for SIGHUP
static void sighup_handler(int sig) {
    sighup_received = 1;
    time_to_move = 1;
    if (in_search) {
        siglongjmp(env, 1);
    }
}

// Signal handler for SIGALRM
static void sigalrm_handler(int sig) {
    time_to_move = 1;
    if (in_search) {
        siglongjmp(env, 1);
    }
}

void engine(Board *bp)
{
    // Set up signal handlers
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction SIGHUP");
        _exit(EXIT_FAILURE);
    }

    sa.sa_handler = sigalrm_handler;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction SIGALRM");
        _exit(EXIT_FAILURE);
    }

    Board *board = newbd();
    copybd(bp, board);

    int our_turn = 0;
    int depth_completed = 0;

    // Announce that we're ready
    printf("Engine ready\n");
    fflush(stdout);

    while (1) {
        // Check if we've been signaled before starting/continuing search
        if (sighup_received) {
            sighup_received = 0;
            goto process_command;
        }

        // Search loop - iteratively deepen search
        for (depth = depth_completed + 1; depth <= MAXPLY; depth++) {
            time_to_move = 0;

            // If it's our turn and avgtime is 0, only search to depth 1
            if (our_turn && avgtime == 0 && depth > 1) {
                break;
            }

            // If it's our turn and we need to move within time limit
            if (our_turn && avgtime > 0) {
                // If avgtime is 0, only search to depth 1 and move immediately
                if (avgtime == 0 && depth > 1) {
                    fprintf(stderr, "[ENGINE] avgtime=0, stopping after depth 1\n");
                    break;
                }
                
                // If avgtime > 0, use time control
                if (avgtime > 0) {
                    // Calculate time available for this move
                    int moves_made = move_number(board);
                    int total_time_used = (player_to_move(board) == X) ? xtime : otime;
                    int avg_time_per_move = (moves_made > 0) ? (total_time_used / moves_made) : 0;
                    int time_available = avgtime - avg_time_per_move;

                    // Check if we have time to search deeper
                    if (depth > 1 && times[depth] > time_available) {
                        break;
                    }

                    // Set alarm for time limit
                    struct itimerval timer;
                    timer.it_value.tv_sec = time_available;
                    timer.it_value.tv_usec = 0;
                    timer.it_interval.tv_sec = 0;
                    timer.it_interval.tv_usec = 0;
                    setitimer(ITIMER_REAL, &timer, NULL);
                }
            }

            // Perform search at current depth
            if (verbose) {
                fprintf(stderr, "Searching depth %d...", depth);
                fflush(stderr);
            }

            reset_stats();

            // Use sigsetjmp to allow escape from bestmove if interrupted
            in_search = 1;
            if (sigsetjmp(env, 1) == 0) {
                int score = bestmove(board, player_to_move(board), 0, principal_var, -MAXEVAL, MAXEVAL);

                in_search = 0;

                // Update timing estimates
                timings(depth);
                depth_completed = depth;

                // Print search information if verbose
                if (verbose) {
                    print_stats();
                    print_pvar(board, depth);
                    fprintf(stderr, "\n");
                }

                // Stop searching if we found a winning or losing position
                if (score == -(MAXEVAL-1) || score == MAXEVAL-1) {
                    break;
                }
            } else {
                // Interrupted by signal - stop searching and process command
                in_search = 0;
                if (verbose) {
                    fprintf(stderr, " interrupted\n");
                }
                break;
            }

            // Cancel alarm if set
            if (our_turn && avgtime > 0) {
                struct itimerval timer;
                timer.it_value.tv_sec = 0;
                timer.it_value.tv_usec = 0;
                timer.it_interval.tv_sec = 0;
                timer.it_interval.tv_usec = 0;
                setitimer(ITIMER_REAL, &timer, NULL);
            }

            // Check if we were interrupted and need to process a command
            if (sighup_received) {
                sighup_received = 0;
                break;
            }
        }

        // Wait for SIGHUP if we haven't received one yet
        if (!sighup_received) {
            while (!sighup_received) {
                pause();
            }
        }
        sighup_received = 0;

process_command:

        // Read command from stdin
        char line[256];
        if (fgets(line, sizeof(line), stdin) == NULL) {
            // EOF - exit
            _exit(EXIT_SUCCESS);
        }

        if (line[0] == '<') {
            // Request to generate a move
            our_turn = 1;

            // Make sure we have completed at least 1-ply search
            if (depth_completed < 1) {
                depth = 1;
                reset_stats();
                bestmove(board, player_to_move(board), 0, principal_var, -MAXEVAL, MAXEVAL);
                timings(1);
                depth_completed = 1;
            }

            // Send the best move
            Move best = principal_var[0];
            print_move(board, best, stdout);
            printf("\n");
            fflush(stdout);

            // Apply the move to our board
            apply(board, best);
            our_turn = 0;

            // Adjust depth_completed if principal variation is still valid
            if (depth_completed > 0) {
                // Shift principal variation down
                for (int i = 0; i < depth_completed - 1; i++) {
                    principal_var[i] = principal_var[i + 1];
                }
                depth_completed--;
            }

        } else if (line[0] == '>') {
            // Opponent's move received
            Move m = read_move_from_pipe(stdin, board);
            if (m == 0) {
                _exit(EXIT_SUCCESS);
            }

            // Apply opponent's move
            apply(board, m);

            // Check if this matches our predicted move
            if (depth_completed > 0 && m == principal_var[0]) {
                // Principal variation is still valid, shift it
                for (int i = 0; i < depth_completed - 1; i++) {
                    principal_var[i] = principal_var[i + 1];
                }
                depth_completed--;
            } else {
                // Principal variation is no longer valid
                depth_completed = 0;
            }
        }
    }

    _exit(EXIT_SUCCESS);
}
