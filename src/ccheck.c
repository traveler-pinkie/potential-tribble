#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#include "ccheck.h"
#include "debug.h"

// Define NO_PLAYER since it's not in the header
#define NO_PLAYER ((Player)-1)

// Define global variable verbose (defined here instead of in library)
int verbose = 0;

// Global variables for signal handling
static volatile sig_atomic_t sigchld_received = 0;
static volatile sig_atomic_t sigint_received = 0;
static volatile sig_atomic_t sighup_received = 0;
static volatile sig_atomic_t sigpipe_received = 0;
static volatile sig_atomic_t sigterm_received = 0;

static pid_t display_pid = 0;
static pid_t engine_pid = 0;

// Signal handlers
static void sigchld_handler(int sig) {
    sigchld_received = 1;
}

static void sigint_handler(int sig) {
    sigint_received = 1;
}

static void sighup_handler(int sig) {
    sighup_received = 1;
}

static void sigpipe_handler(int sig) {
    sigpipe_received = 1;
}

static void sigterm_handler(int sig) {
    sigterm_received = 1;
}

// Helper function to kill child processes
static void cleanup_processes() {
    if (display_pid > 0) {
        kill(display_pid, SIGKILL);
        display_pid = 0;
    }
    if (engine_pid > 0) {
        kill(engine_pid, SIGKILL);
        engine_pid = 0;
    }
}

/*
 * Options (see the assignment document for details):
 *   -w           play white
 *   -b           play black
 *   -r           randomized play
 *   -v           give info about search
 *   -d           don't try to use X window system display
 *   -t           tournament mode
 *   -a <num>     set average time per move (in seconds)
 *   -i <file>    initialize from saved game score
 *   -o <file>    specify transcript file name
 */

int ccheck(int argc, char *argv[])
{
    int option;
    Player engine_player = NO_PLAYER;
    int no_display = 0;
    int tournament_mode = 0;
    char *init_file = NULL;
    char *output_file = NULL;
    FILE *transcript = NULL;

    // Parse command-line arguments
    while((option = getopt(argc, argv, "wbrvdta:i:o:")) != -1){
        switch(option){
            case 'w':
                engine_player = X;
                break;
            case 'b':
                engine_player = O;
                break;
            case 'r':
                randomized = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'd':
                no_display = 1;
                break;
            case 't':
                tournament_mode = 1;
                break;
            case 'a':
                avgtime = atoi(optarg);
                break;
            case 'i':
                init_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case ':':
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            case '?':
                fprintf(stderr, "Unknown option: -%c.\n", optopt);
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // Set up signal handlers
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = sighup_handler;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction SIGHUP");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = sigpipe_handler;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction SIGPIPE");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = sigchld_handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(EXIT_FAILURE);
    }

    // Create the board
    Board *board = newbd();

    // Set up pipes for display process
    int display_to_main[2];
    int main_to_display[2];
    FILE *display_in = NULL;
    FILE *display_out = NULL;

    // Start display process if not disabled
    if (!no_display) {
        if (pipe(display_to_main) == -1 || pipe(main_to_display) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        display_pid = fork();
        if (display_pid == -1) {
            perror("fork display");
            exit(EXIT_FAILURE);
        }

        if (display_pid == 0) {
            // Child process - run xdisp
            close(display_to_main[0]);  // Close read end in child
            close(main_to_display[1]);  // Close write end in child

            // Redirect stdin/stdout
            if (dup2(main_to_display[0], STDIN_FILENO) == -1) {
                perror("dup2 display stdin");
                _exit(EXIT_FAILURE);
            }
            if (dup2(display_to_main[1], STDOUT_FILENO) == -1) {
                perror("dup2 display stdout");
                _exit(EXIT_FAILURE);
            }

            close(main_to_display[0]);
            close(display_to_main[1]);

            // Execute xdisp
            execlp("util/xdisp", "xdisp", NULL);
            perror("execlp xdisp");
            _exit(EXIT_FAILURE);
        }

        // Parent process
        close(display_to_main[1]);  // Close write end in parent
        close(main_to_display[0]);  // Close read end in parent

        display_in = fdopen(display_to_main[0], "r");
        display_out = fdopen(main_to_display[1], "w");

        if (!display_in || !display_out) {
            perror("fdopen display");
            cleanup_processes();
            exit(EXIT_FAILURE);
        }

        // Wait for display to be ready
        char ready_line[256];
        if (fgets(ready_line, sizeof(ready_line), display_in) == NULL) {
            fprintf(stderr, "Display process failed to initialize\n");
            cleanup_processes();
            exit(EXIT_FAILURE);
        }
    }

    // Open transcript file if specified
    if (output_file) {
        transcript = fopen(output_file, "w");
        if (!transcript) {
            perror("fopen transcript");
            cleanup_processes();
            exit(EXIT_FAILURE);
        }
    }

    // Read initial game history if specified
    if (init_file) {
        FILE *init = fopen(init_file, "r");
        if (!init) {
            perror("fopen init file");
            cleanup_processes();
            if (transcript) fclose(transcript);
            exit(EXIT_FAILURE);
        }

        Move m;
        while ((m = read_move_from_pipe(init, board)) != 0) {
            Player current_player = player_to_move(board);
            apply(board, m);
            
            // Log to transcript if specified (same logic as main game loop)
            if (transcript) {
                int move_num = move_number(board);
                if (current_player == X) {
                    // White move: move_num will be odd (1,3,5...), display as (1,2,3...)
                    fprintf(transcript, "%d. ", (move_num / 2) + 1);
                } else {
                    // Black move: move_num will be even (2,4,6...), display as (1,2,3...)
                    fprintf(transcript, "%d. ... ", move_num / 2);
                }
                print_move(board, m, transcript);
                fprintf(transcript, "\n");
                fflush(transcript);
            }

            // Update display if active
            if (!no_display && display_out) {
                fprintf(display_out, ">");
                print_move(board, m, display_out);
                fprintf(display_out, "\n");
                fflush(display_out);
                kill(display_pid, SIGHUP);

                // Wait for acknowledgement
                char ack[256];
                if (fgets(ack, sizeof(ack), display_in) == NULL) {
                    fprintf(stderr, "Display died during init\n");
                    cleanup_processes();
                    if (transcript) fclose(transcript);
                    exit(EXIT_FAILURE);
                }
            }
        }
        fclose(init);
    }

    // Set up pipes for engine process
    int engine_to_main[2];
    int main_to_engine[2];
    FILE *engine_in = NULL;
    FILE *engine_out = NULL;

    // Start engine process if needed
    if (engine_player != NO_PLAYER) {
        if (pipe(engine_to_main) == -1 || pipe(main_to_engine) == -1) {
            perror("pipe");
            cleanup_processes();
            if (transcript) fclose(transcript);
            exit(EXIT_FAILURE);
        }

        engine_pid = fork();
        if (engine_pid == -1) {
            perror("fork engine");
            cleanup_processes();
            if (transcript) fclose(transcript);
            exit(EXIT_FAILURE);
        }

        if (engine_pid == 0) {
            // Child process - run engine
            close(engine_to_main[0]);  // Close read end in child
            close(main_to_engine[1]);  // Close write end in child

            // Redirect stdin/stdout
            if (dup2(main_to_engine[0], STDIN_FILENO) == -1) {
                perror("dup2 engine stdin");
                _exit(EXIT_FAILURE);
            }
            if (dup2(engine_to_main[1], STDOUT_FILENO) == -1) {
                perror("dup2 engine stdout");
                _exit(EXIT_FAILURE);
            }

            close(main_to_engine[0]);
            close(engine_to_main[1]);

            // Call engine function
            engine(board);
            _exit(EXIT_SUCCESS);
        }

        // Parent process
        close(engine_to_main[1]);  // Close write end in parent
        close(main_to_engine[0]);  // Close read end in parent

        engine_in = fdopen(engine_to_main[0], "r");
        engine_out = fdopen(main_to_engine[1], "w");

        if (!engine_in || !engine_out) {
            perror("fdopen engine");
            cleanup_processes();
            if (transcript) fclose(transcript);
            exit(EXIT_FAILURE);
        }

        // Wait for engine to be ready
        char ready_line[256];
        if (fgets(ready_line, sizeof(ready_line), engine_in) == NULL) {
            fprintf(stderr, "Engine process failed to initialize\n");
            cleanup_processes();
            if (transcript) fclose(transcript);
            exit(EXIT_FAILURE);
        }
    }

    // Main game loop
    int game_over_flag = 0;
    int error_occurred = 0;
    while (!game_over_flag) {
        // Check for termination signals
        if (sigint_received || sigpipe_received || sigterm_received) {
            if (sigpipe_received || sigterm_received) {
                error_occurred = 1;
            }
            break;
        }

        if (sigchld_received) {
            sigchld_received = 0;
            int status;
            pid_t pid = waitpid(-1, &status, WNOHANG);
            if (pid == display_pid || pid == engine_pid) {
                fprintf(stderr, "Child process died unexpectedly\n");
                error_occurred = 1;
                break;
            }
        }

        Player current_player = player_to_move(board);
        Move move = 0;

        // Determine how to get the move
        if (current_player == engine_player && engine_in && engine_out) {
            // Engine's turn - request move
            fprintf(engine_out, "<\n");
            fflush(engine_out);
            kill(engine_pid, SIGHUP);

            // Read move from engine
            move = read_move_from_pipe(engine_in, board);
            if (move == 0) {
                fprintf(stderr, "Engine died\n");
                error_occurred = 1;
                break;
            }

            // Charge engine for time
            setclock(current_player);

        } else if (!no_display && !tournament_mode && display_in && display_out) {
            // Interactive mode with display - request move from display
            fprintf(display_out, "<\n");
            fflush(display_out);
            kill(display_pid, SIGHUP);

            // Read move from display
            move = read_move_from_pipe(display_in, board);
            if (move == 0) {
                fprintf(stderr, "Display died\n");
                error_occurred = 1;
                break;
            }

            // Charge player for time
            setclock(current_player);

        } else {
            // Interactive mode from stdin
            move = read_move_interactive(board);
            if (move == 0) {
                // EOF
                break;
            }

            // Charge player for time
            setclock(current_player);
        }

        // Apply the move
        apply(board, move);

        // Print move - in tournament mode, prefix computer moves with @@@
        if (tournament_mode && current_player == engine_player) {
            printf("@@@");
        }
        print_move(board, move, stdout);
        printf("\n");
        fflush(stdout);

        // Update display if active
        if (!no_display && display_out) {
            fprintf(display_out, ">");
            print_move(board, move, display_out);
            fprintf(display_out, "\n");
            fflush(display_out);
            kill(display_pid, SIGHUP);

            // Wait for acknowledgement
            char ack[256];
            if (fgets(ack, sizeof(ack), display_in) == NULL) {
                fprintf(stderr, "Display died\n");
                error_occurred = 1;
                break;
            }
        }

        // Print board if no display
        if (no_display) {
            print_bd(board, stdout);
        }

        // Log to transcript  (note: this is AFTER apply(), so move_num has been incremented)
        if (transcript) {
            int move_num = move_number(board);
            if (current_player == X) {
                // White move: move_num will be odd (1,3,5...), display as (1,2,3...)
                fprintf(transcript, "%d. ", (move_num / 2) + 1);
            } else {
                // Black move: move_num will be even (2,4,6...), display as (1,2,3...)
                fprintf(transcript, "%d. ... ", move_num / 2);
            }
            print_move(board, move, transcript);
            fprintf(transcript, "\n");
            fflush(transcript);
        }

        // Send move to engine if it's the opponent's move
        if (current_player != engine_player && engine_player != NO_PLAYER && engine_out) {
            fprintf(engine_out, ">");
            print_move(board, move, engine_out);
            fprintf(engine_out, "\n");
            fflush(engine_out);
            kill(engine_pid, SIGHUP);
            
            // Wait for acknowledgement
            char ack[256];
            if (fgets(ack, sizeof(ack), engine_in) == NULL) {
                fprintf(stderr, "Engine died\n");
                error_occurred = 1;
                break;
            }
        }

        // Check if game is over
        int winner = game_over(board);
        if (winner != 0) {
            if (winner == 1) {
                printf("White wins!\n");
            } else {
                printf("Black wins!\n");
            }
            game_over_flag = 1;

            // Wait for termination signal
            while (!sigint_received && !sigchld_received && !sigterm_received) {
                pause();
            }
            break;
        }
    }

    // Cleanup
    cleanup_processes();
    if (display_in) fclose(display_in);
    if (display_out) fclose(display_out);
    if (engine_in) fclose(engine_in);
    if (engine_out) fclose(engine_out);
    if (transcript) fclose(transcript);
    
    // Free board memory
    free(board);

    // Wait for any remaining children
    while (waitpid(-1, NULL, WNOHANG) > 0);

    return error_occurred ? EXIT_FAILURE : EXIT_SUCCESS;
}
