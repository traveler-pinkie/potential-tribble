# Homework 4 Multi-Process Game Engine - CSE 320 - Fall 2025
#### Professor Eugene Stark

### **Due Date: Friday 11/14/2025 @ 11:59pm**

## Introduction

The goal of this assignment is to become familiar with low-level Unix/POSIX system
calls related to processes, signal handling, files, and I/O redirection.
You will implement portions of a multi-process program, called `ccheck`,
that plays the game of "Chinese Checkers"
(see, e.g. [Wikipedia](https://en.wikipedia.org/wiki/Chinese_checkers)).
Your implementation will include the top-level driver that performs argument
processing and coordinates the play of the game, as well as a control portion
for the "engine" that generates moves when playing against the computer.

Since this assignment is supposed to be about processes, signals, and the like,
binary modules have been provided for you that handle many of the other details
specifically involved in playing the game, such as move parsing, maintaining the
game board, and searching the game tree to try to find the best move.
The entire implementation, including the multi-process organization, is based on
code that I originally wrote in the 1989-1991 time frame to run on the HP-UX
and SunOS operating systems.  I have updated that code so that it now works on a
modern Linux system, and refactored it for this assignment so that you can focus
on the process-manipulation part without getting distracted by other details.

### Takeaways

After completing this assignment, you should:

* Understand process execution: forking, executing, and reaping.
* Understand signal handling.
* Understand the use of "dup" to perform I/O redirection.
* Have a more advanced understanding of Unix commands and the command line.
* Have gained experience with C libraries and system calls.
* Have enhanced your C programming abilities.

## Hints and Tips

* We **strongly recommend** that you check the return codes of **all** system calls
  and library functions.  This will help you catch errors.
* **BEAT UP YOUR OWN CODE!** Exercise your code thoroughly with various timing situations,
  to make sure that no sequence of events can occur that can crash the program.
* Your code should **NEVER** crash, and we will deduct points every time your
  program crashes during grading.  Especially make sure that you have avoided
  race conditions involving signal handling that might result in "flaky" behavior.
  If you notice odd behavior you don't understand: **INVESTIGATE**.
* You should use the `debug` macro provided to you in the base code.
  That way, when your program is compiled without `-DDEBUG`, all of your debugging
  output will vanish, preventing you from losing points due to superfluous output.

> :nerd: When writing your program, try to comment as much as possible and stay
> consistent with code formatting.  Keep your code organized, and don't be afraid
> to introduce new source files if/when appropriate.

### Reading Man Pages

This assignment will involve the use of many system calls and library functions
that you probably haven't used before.
As such, it is imperative that you become comfortable looking up function
specifications using the `man` command.

The `man` command stands for "manual" and takes the name of a function or command
(programs) as an argument.
For example, if I didn't know how the `fork(2)` system call worked, I would type
`man fork` into my terminal.
This would bring up the manual for the `fork(2)` system call.

> :nerd: Navigating through a man page once it is open can be weird if you're not
> familiar with these types of applications.
> To scroll up and down, you simply use the **up arrow key** and **down arrow key**
> or **j** and **k**, respectively.
> To exit the page, simply type **q**.
> That having been said, long `man` pages may look like a wall of text.
> So it's useful to be able to search through a page.
> This can be done by typing the **/** key, followed by your search phrase,
> and then hitting **enter**.
> Note that man pages are displayed with a program known as `less`.
> For more information about navigating the `man` pages with `less`,
> run `man less` in your terminal.

Now, you may have noticed the `2` in `fork(2)`.
This indicates the section in which the `man` page for `fork(2)` resides.
Here is a list of the `man` page sections and what they are for.

| Section          | Contents                                |
| ----------------:|:--------------------------------------- |
| 1                | User Commands (Programs)                |
| 2                | System Calls                            |
| 3                | C Library Functions                     |
| 4                | Devices and Special Files               |
| 5                | File Formats and Conventions            |
| 6                | Games et. al                            |
| 7                | Miscellanea                             |
| 8                | System Administration Tools and Daemons |

From the table above, we can see that `fork(2)` belongs to the system call section
of the `man` pages.
This is important because there are functions like `printf` which have multiple
entries in different sections of the `man` pages.
If you type `man printf` into your terminal, the `man` program will start looking
for that name starting from section 1.
If it can't find it, it'll go to section 2, then section 3 and so on.
However, there is actually a Bash user command called `printf`, so instead of getting
the `man` page for the `printf(3)` function which is located in `stdio.h`,
we get the `man` page for the Bash user command `printf(1)`.
If you specifically wanted the function from section 3 of the `man` pages,
you would enter `man 3 printf` into your terminal.

> :scream: Remember this: **`man` pages are your bread and butter**.
> Without them, you will have a very difficult time with this assignment.

## Getting Started

Fetch and merge the base code for `hw4` as described in `hw0`.
You can find it at this link: https://gitlab02.cs.stonybrook.edu/cse320/hw4

Here is the structure of the base code:
<pre>
.
├── .gitlab-ci.yml
└── hw4
    ├── demo
    │   ├── ccheck
    │   ├── ccheck.d
    │   └── ccheck_db
    ├── .gitignore
    ├── hw4.sublime-project
    ├── include
    │   ├── ccheck.h
    │   └── debug.h
    ├── lib
    │   └── ccheck.a
    ├── Makefile
    ├── src
    │   ├── ccheck.c
    │   ├── engine.c
    │   └── main.c
    └── util
        ├── versus
        └── xdisp
</pre>

As usual, the `include` directory contains header files and the `src`
directory contain C source files.  The `ccheck.h` header file that has
been provided in the base code should not be modified.  You may, however,
create your own header files if you wish.  Also, the `main.c` source file should
not be changed.  The `demo` directory contains two demonstration versions of
the completed program.  One version (`ccheck_db`) produces debugging
printout that will help you to understand the execution.  The other version
(`ccheck`) does not produce debugging printout.
The `lib` directory contains a library `ccheck.a` that will be linked with your
code to supply various pre-implemented functions that are not supposed to
be the focus of this assignment.  Except for the interface to the things that
you will have to use, much of the content is not documented.
The `ccheck.h` header file, however, contains documentation for the library
functions and global variables that you will actually need to use.

> :scream: To avoid doing unnecessary work, you should definitely make a point
> of reading the `ccheck.h` header file right away and familiarizing yourself
> with the functions provided.  You should *not* be spending time doing
> things like parsing moves, as functions for this have been provided for you.

The `util` directory contains two stand-alone programs: `xdisp` and `versus`.
The `xdisp` program contains a very simple (and feature-poor) graphical
interface for the game board.  As discussed below, the main program will
need to start a separate process to execute this program and it will need
to interact with that process in order to keep the board display up-to-date and to
accept moves from the user.
The `versus` program is a stand-alone driver program that permits two instances
of `ccheck` to compete against each other over a network connection.
The use of this program will be discussed further below.

The `src` directory contains three source files: `main.c`, `ccheck.c`,
and `engine.c`.  The `main.c` file is just an implementation of `main()`
that calls the function `ccheck` in `ccheck.c`.  You should not modify
the `main.c` file.  The `ccheck.c` file contains a stub for the `ccheck`
function, which you have to implement before the game will function.
The `engine.c` file likewise contains a stub for the game "engine".
This stub has been completely commented out with `#if 0`/`#endif`,
so that by default it will not result in any object code.  In that case,
a demonstration version of the `engine` function will be linked from
the `ccheck.a` library.  This will let you work on the main program first,
before attempting to implement the engine.  When you are ready to implement
the engine, you can uncomment the stub and provide your own implementation,
which will take precedence over the library version.
There are also some comments in the `engine.c` file that give a sketch of
how the implementation can look and point you at some functions and variables
that you need to know about in order to satisfy the expectations of the
underlying library functions.

If you run `make` on the unmodified basecode, the code should compile correctly,
resulting in an executable `bin/ccheck`.  If you run this program, it will
just print a message and abort.  This is because you at least have to
implement the `ccheck` function in `ccheck.c` before the game will function.

## 'Ccheck': Functional Specification

Invoking `ccheck` from the command line results in the function `ccheck` in
`ccheck.c` being called with the `argc` and `argv` that was passed to `main`.
We will refer to the process running `ccheck` as the "main process".
The function of the main process is to serve as a referee for a game between
two players (one of which may be the computer) and to transfer moves between
interactive user(s), the graphical display process, and the engine process.
It maintains its own idea of the game state and will prompt the interactive
user or communicate with the display process or the engine process to obtain
or display moves and allow the game to progress.  It also detects when the game
is over and pauses until termination is subsequently triggered by a signal from
the terminal or the termination of either the display or the engine process.

### Command-Line Arguments

The `ccheck` program accepts the following command-line arguments:

```c
/*
 * Options:
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
```

   * If `-w` is given, then the engine will be used to generate moves for White
     (the first player to move).

   * If `-b` is given, then the engine will be used to generate moves for Black
     (the second player to move).

   * If neither `-w` nor `-b` is given, then the engine will not be started and
     the program performs as a referee for two interactive users.

   * The `-r` flag is used to specify that the engine should use randomization so
      that it does not always make the same move in the same position.  The setting
      is accomplished by assigning a nonzero value to the `randomized` global
      variable declared in the `ccheck.h` header file.

   * If `-v` is given, then if an engine process is used, it will print out statistics
      about the search it is performing.  The setting is accomplished by assigning
      a nonzero value to the `verbose` global variable declared in `ccheck.h`

   * By default, the program starts a process running "xdisp" to display the
      game board and accept moves.  If the `-d` flag is given, then the use of
      the graphical display is disabled and the program instead uses an
      ASCII board printed to the standard output.

   * If `-t` is given, then the program functions in "tournament mode".
      In tournament mode, the program will accept moves either from the engine
      (if `-w` or `-b` has been given) or via interactive input on the standard input.
      If the graphical display is used (the default unless `-d` has been given)
      then it is used only to show the current board position, not to input moves.

      Normally, tournament mode would be used with the external driver program
      `versus`, which allows two competitors to play each other over a network
      connection.
      Moves made by the computer in tournament mode are printed to the standard output,
      as would be the case for interactive play with a human user, but in addition
      each move is prefixed by the sequence `@@@`.  This allows the `versus` program
      to detect moves within otherwise arbitrary output, so that they can be sent
      to the opponent program.

   * The `-a` flag is used to specify the average time per move (in seconds) that the
      engine will take.  If `0` is given, then the engine will move "as soon as
      possible".  The setting is accomplished by assigning the specified value
      to the `avgtime` global variable (declared in `ccheck.h`).

   * The `-o` flag is used to cause a transcript of the game to be written to a file
      for posterity.  An example of the format is as follows:

```
        1. white:A3-C3
        1. ... black:I7-G7
        2. white:C3-C4
        2. ... black:H8-F8
        3. white:A1-A3-C3-C5
        3. ... black:G8-G6
        4. white:C5-D5
        4. ... black:G7-F7
        5. white:C1-C3-C5-E5
```

   * The `-i` flag is used to initialize the board state from a previously
      saved game score.  The specified file is opened and moves (one per line)
      are read from the file.
      As the game history is read, the display is updated and once the entire
      history has been read the game will resume from the resulting position.

### Behavior of the Main Process

   When the main program is invoked, it first performs argument processing.
   After that, it sets up handlers for `SIGINT`, `SIGHUP`, `SIGPIPE`,
   and `SIGTERM` so that in the event any of these signals are received,
   a graceful shutdown can be performed after killing any remaining child processes
   (display, engine, or both).

   Next, if the graphical display is to be used (the default unless `-d` has
   been specified), a child process is created to run the `util/xdisp` program.
   The main process will interact with the display process using pipes and a
   signalling protocol which is described below.

   If `-i` has been given, the partial game history in the specified file is then
   read and the board position is updated by the moves in this history.
   If the graphical display is in use, it is kept updated as the moves are read.
   If `-o` has been given, then the moves read are also logged to the specified
   output file.

   Next, if the engine is to be used (*i.e.* if the computer is to be one of the players),
   then a child process is created for this purpose, and it is arranged for the
   child process to call the `engine` function, passing to it the game board
   in the state that has resulted from reading any partial game history.

   Finally, the main game loop is started.
   For each move, depending on the player whose turn it is and the options selected,
   a move is obtained: either interactively, from the graphical display, or from
   the engine.  The move is applied to the game board and the graphical display
   is updated if it is in use, otherwise a representation of the game board showing
   the new position is printed to stdout.  If the engine is in use, and the move came
   from the opponent, then the move is sent to the engine so that it may calculate
   its response.

   After each move is made, it is checked whether the game has been won by one of the
   players.  If so, then the play loop is terminated, the program announces who has
   won, and the program then pauses until either `SIGINT` is received (indicating that
   the user has requested termination) or a notification is received (via `SIGCHLD`
   or an `EOF` condition on a pipe used for communication with a child process) that
   one of the child processes has terminated.  Any remaining child processes are then
   killed, any log file in use is closed, and the program then exits with status
   `EXIT_SUCCESS`.

   Any errors or abnormal events that occur during execution cause the program
   to issue an error message to the standard error output, to kill any existing child
   processes, and to exit with status `EXIT_FAILURE`.

### Communication with the Graphical Display Process

   Communication with the display process (`xdisp`) occurs over a pair of pipes using
   a signalling protocol.  As part of the invocation of the display process,
   two pipes are created.  One of these is used to send data from the main program
   to the standard input of the display process and the other is used by the main
   process to receive data sent by the display process to its standard output.

   When the display process first starts and has completed its initialization,
   it will prints a single line to standard output to announce to the main process that
   it is now ready for interaction.  After launching the display process,
   the main process must wait for this announcement before proceeding.
   This synchronization is necessary because once created, a child process runs
   concurrently with its parent and the parent cannot know for sure at what point
   the child process has fully initialized and can be sent signals, *etc.*

   When the main program wants to inform the display about a move that has been made,
   it prints on the pipe to the display process a single line, in the following format:

```
   >white:A3-C3
```

   The line begins with a single `>` character, is immediately followed by the move
   in the format illustrated, and is terminated with single newline character.
   There should be no additional whitespace or other characters added to the line.
   Once the line has been sent, the main program sends a `SIGHUP` signal to the display
   process.  Upon receipt of the `SIGHUP` signal, the display process will read the
   move that has been sent by the main process and the display will be updated.
   The display process then prints a single-line acknowledgement on its standard output.
   The main process must wait to read this acknowledgement from the pipe before proceeding.

   If the display is being used to input moves from the user (this will be the case
   unless either option `-d` (no display) or `-t` (tournament mode) has been given),
   when it is time for the user to move then the main process will request a move
   from the display process by first outputting on the pipe to the display process
   a single line of the form

```
   <
```

   consisting of a `<` character followed immediately by a single newline character.
   The main process then sends `SIGHUP` to the display process.  The display process
   will respond by printing a move to the standard output, followed by a single
   newline character.  The main process will then read the move from the pipe and
   perform any required processing.

### The 'Engine' Process

   The 'engine' is the module responsible for determining the computer's move.
   It provides one function: `engine(Board *)`, which is intended to be invoked by
   a child process of the main process.  The board that is passed should contain the
   starting position for the game, after any partial history read from a file specified
   using `-i` has been applied.

   The engine process spends its time searching for the best move it can find from
   the current position.  It performs this searching not only "on its own time";
   *i.e.* when it is the computer's turn to move, but also "on the opponent's time";
   *i.e.* when it is the opponent's turn to move.  When searching on its own time,
   the depth to which the game tree is searched is limited in order to stay within
   a specified average time per move over the course of the entire game.  This is done by
   keeping track of how long it has taken in the past to search the game tree to
   each particular depth, and to avoid embarking on a search if it is estimated that
   performing that search will put the average time per move over the limit.
   The limit is specified as an average over the entire game; this is so that if
   not much time was spent on previous moves, then more time will be available for
   use on later moves.  When the engine is searching on its opponent's time,
   there is no time limit and it will simply continue to search the game tree at
   deeper and deeper depths, up until it has completed a search at a fixed maximum
   depth (specified at compile time), or else the search is terminated by the
   receipt of a `SIGHUP` signal from the main process, indicating that it is now
   the engine's turn to move.

   Technically, the algorithm used by the engine to search for a move is called
   "minimax, with alpha/beta pruning".
   If you are interested, you can find more information
   [here](https://en.wikipedia.org/wiki/Minimax), but for the purposes of this assignment
   you don't need to have a deep understanding of it.
   The interface to the search algorithm is provided by the function `bestmove`,
   which is called to search the game tree to a lookahead depth specified as a number
   of 'plies'; where each 'ply' corresponds to a move made by one of the players.
   The depth cutoff is specified by setting the global variable `depth` to the desired
   number of plies (yes, it probably ought to have been an argument instead of a
   global variable, but I got tired of refactoring the original program).
   When `bestmove` completes, it fills in the `principal_var` array with the so-called
   "principal variation", which consists of a sequence of best moves for each player,
   starting from the current board position and leading to a "leaf" position
   at which a "static evaluation function" has been applied to determine the relative
   "goodness" of that position (positive = "good", negative = "bad") for the player
   who has just moved.  If `bestmove` is called with `depth` set to value d,
   then upon return the entries at indices 0, 1, ..., d-1 in the `principal_var` array
   will be valid.  The best move found for the player who is to move in the current
   board position will be in `principal_var[0]`, so it is important that a search
   to a depth of at least 1 ply has been completed before attempting to obtain a move.

   There are several constants and global variables involved in the interaction
   of the `engine` function with `bestmove`; these are documented in `ccheck.h`.
   The variables `depth` and `randomized` modify the behavior of `bestmove`.
   The variable `principal_var` is set by `bestmove` to the principal variation,
   as already discussed.  The entries of the `times` array are updated by the `timings`
   function to contain current estimates of the number of seconds that it will take to
   search the current game tree to the specified depth.  The `timings` function should
   be called each time a call to `bestmove` has completed.
   The `engine` function should use the resulting timing estimates to attempt to avoid
   embarking on a search that will take too long to complete, if it is currently the
   computer's turn to move.  The `avgtime` variable is set in the `ccheck` function
   and should be used by the `engine` function as a target on how much time it can
   afford to spend on a search; it is not used or set by `bestmove` itself.
   There are some other variables: namely `searchtime`, `movetime`, `xtime`, and `otime`,
   which are set by `bestmove` and are printed out by the `print_stats` function to
   give information about the searching, but these are not used to actually control
   the search.
   The variables `verbose` is set by the `ccheck` function and should be used
   by the `engine` function to determine whether these statistics are printed (to `stderr`)
   while searching is active.
   
   The stub provided in `engine.c` contains a rough skeleton of how the code for
   the `engine` function might be organized.  The main purpose of providing this
   is as a guide to what should be done with the various global variables and
   library functions in order to interact properly with the searching functions.
   It is up to you to flesh out the details and make it work. Your concern should
   primarily be the communication and control structure, including pipes,
   signal handling and nonlocal jumps.
   The functions for actually searching the game tree, maintaining statistics,
   input and output of moves, etc. have been provided in binary form for your use,
   so that you don't get distracted by trying to implement things that are not
   the main focus of the assignment.

### Communication with the Engine Process

   Communication between the main process and the engine process follows the same
   protocol pattern as for communication between the main process and the display.
   Specifically, when you launch the engine process you will need to set up two
   pipes for communicating with it: one connected to the standard input of the
   engine process and the other connected to its standard output.
   The format of what is sent over the pipes is the same as for the display process,
   and the same `SIGHUP` protocol is to used.
   Note that, as the engine may be searching on its opponent's time, the `SIGHUP`
   serves an essential purpose of interrupting any search that is in progress and
   informing the engine process that an opponent's move is available and that
   it is the engine's turn to move.

## The 'versus' Driver Program

A stand-alone program `versus` (in the `util` directory) has been provided with
the basecode.  The purpose of this program is to allow two instances of `ccheck`
to play against each other via a network connection.  The two instances could
both be running on the same computer, or they could be running remotely from
each other.  Assuming that the required network access is not being blocked by
a firewall, you would be able to use this program to have your program play against
somebody else's, even if you are not in the same physical location.

In order to use `versus`, you first run it on the computer that will be used
by the first player to move.  You provide the command-line argument `-w`
to `versus` to specify this.  The `-w` is followed on the same command line by the
name of the `ccheck` program and arguments to it.  One argument that you need to
give to `ccheck` is the `-t` argument to select tournament mode.
This forces `ccheck` to use the standard
input and standard output to accept and issue moves and to use the graphical
display (if selected) only to show the current board position.
If the `ccheck` program is going to use the engine to generate moves, then
you will need to give the same `-w` flag to `ccheck` as you did to `versus`.

Once the first instance of `versus` has started, it will launch `ccheck`
as a child process and arrange to communicate with it using a mechanism called
"pseudo-terminals" (or "ptys" for short).  In brief, what it does is to look
at what each competitor outputs on its standard output and to transfer that as
input to the standard input of the other.
`Versus` will then start a network server and print out hostname and TCP port
number that you will need to give to the second instance of `versus` in order
for it to make a connection to the first.  You then need to start this second
instance of `versus`, either in a separate terminal window or on another computer,
passing the host and port number specified by the first instance.

For example, suppose you start the first instance of `versus` as follows:

```
$ util/versus -w demo/ccheck -t -w -a 10
```

You should then see the following:

```
Invoking White program...
Using X Window System display.
White program started.
Now execute: util/versus -b bigiron 49963 (Black program command line)
Waiting for connection to Black program...
```

Here "bigiron" is the hostname of the computer on which `versus` was started.
In a separate window (or on a separate computer), you would then type:

```
$ util/versus -b bigiron 49963 demo/ccheck -t -b -a 10
```

The `-b` argument passed to `ccheck` must match the `-b` passed to `versus`,
but aside from that (and the required `-t`) the arguments to the second instance
of `ccheck` need not be the same as those given to the first instance.

> :nerd: The hostname printed out by `versus` will most likely not be a
> fully qualified hostname.  If you are trying to connect from another
> computer, it might be necessary to use a fully qualified name constructed
> by appending the domain name of the first computer (example: `.stonybrook.edu`)
> to the hostname printed by `versus`.  Alternatively, an IPv4 address can be
> given in place of the hostname.

Once you have entered this second command, you will see:

```
Invoking Black program...
Using X Window System display.
Your move? white:A3-C3
@@@black:G9-G7
Your move? white:C3-C4
@@@black:G7-F7
...
```

as the programs begin to play each other.  You can follow the action on the
graphical board display, or (if you didn't want to use that), on the ASCII
boards that are printed out the terminal.


> :nerd: Note that when two `ccheck` programs are playing each other with `versus`,
> they use interactive input and output to the standard input and standard output,
> as if it were two human beings that were playing interactively using a terminal to
> input and output their moves.  The only difference is that each move that is
> made is prefixed by `@@@` to make it easy for `versus` to filter the move lines
> from all the other lines of output that might be getting printed.

## Implementation Notes

### Functions You Must Implement

There just two functions which you are required to implement as specified:

* `int ccheck(int argc, char *argv[])` - This function is called from `main()`,
  which directly passes the `argc` and `argv` that it has received.
  The purpose of the `ccheck` function is to implement the functionality of
  the "main process" as discussed above.

* `void engine(Board *bp)` - This function should be called in a child of the main process,
  in order to implement the "engine process" functionality discussed above.
  In the basecode, this function is commented out, which results in a demo version
  of this function being linked from the `ccheck.a` library.
  If you uncomment this function, then the implementation from your `engine.c`
  will supersede the demo version from the library.

You will of course almost certainly want to implement auxiliary functions
besides these two, but these are the only ones whose interfaces and behavior
are directly specified.

### System Calls and Library Function to Use

It is anticipated that you will want to make use of many of the following
functions from Linux libraries:

`abort`,
`alarm`,
`close`,
`dup`,
`dup2`,
`execvp`,
`execlp`,
`_exit`,
`exit`,
`fclose`,
`fflush`,
`fgetc`,
`fork`,
`fopen`,
`fprintf`,
`fputc`,
`getopt`, 
`kill`,
`pipe`,
`setitimer`, 
`sigaction`,
`sigaddset`,
`sigemptyset`,
`signal`,
`sigprocmask`,
`sigsetjmp`,
`siglongjmp`,
`sigsuspend`,
`waitpid`,

This list probably covers just about everything you will need -- if you find
yourself wanting to use some bizarre function not in this list you should 
step back, reconsider how you got the idea of using that function, and be sure
you understand why you need it.

### Signal Handling

You will need to use signal handling for this program.
If you want your program to work reliably, you must only use async-signal-safe
functions in your signal handler.
You should make every effort not to do anything "complicated" in a signal handler;
rather the handler should just set flags or other variables to communicate back to
the main program what has occurred and the main program should check these flags and
perform any additional actions that might be necessary.
Variables used for communication between the handler and the main program should
be declared `volatile` so that a handler will always see values that
are up-to-date (otherwise, the C compiler might generate code to cache updated
values in a register for an indefinite period, which could make it look to a
handler like the value of a variable has not been changed when in fact it has).
Ideally, such variables should be of type `sig_atomic_t`, which means that they
are just integer flags that are read and written by single instructions.
Note that operations, such as the auto-increment `x++`, and certainly more complex
operations such as structure assignments, will generally not be performed as a single
instruction.  This means that it would be possible for a signal handler to be
invoked "in the middle" of such an operation, which could lead to "flaky"
behavior if it were to occur.

Recall that standard I/O functions such as `fprintf()` are not async-signal-safe,
and thus cannnot reliably be used in signal handlers.  For example, suppose the
main program is in the middle of doing an `fprintf()` when a signal handler is invoked
asynchronously and the handler itself tries to do `fprintf()`.  The two invocations
of `fprintf()` share state (not just the `FILE` objects that are being printed to,
but also static variables used by functions that do output formatting).
The `fprintf()` in the handler can either see an inconsistent state left by the
interrupted `fprintf()` of the main program, or it can make changes to this state that
are then visible upon return to the main program.  Although it can be quite useful
to put debugging printout in a signal handler, you should be aware that you can
(and quite likely will) see anomalous behavior resulting from this, especially
as the invocations of the handlers become more and more frequent.  Definitely be sure
to remove or disable this debugging printout in any "production" version of your
program, or you risk unreliability.

Note that also that, depending on the details of your implementation, you might
need to use `sigprocmask()` to block signals at appropriate times, to avoid races
between the handler and the main program, the occurrence of which can also result
in indeterminate behavior.  In general, signals must be blocked any time the main
program is actively involved in manipulating variables that are shared with a
signal handler.

### Debugging

In case you find that one of your process "hangs" during execution, you might find
it useful to use the capability of `gdb` to attach to a process that is already
running.  This is done by starting `gdb` in the normal way, specifying the name of the
executable object file (for the debugging symbols), but then, once `gdb` has started,
rather than using the `run` command to run the program, you use the `attach` command
to tell `gdb` to attach to the process with a specified process ID.  You will have
to use a command such as `ps` to get the process ID of the running process you want
to debug.  System security restrictions will likely make it necessary to run `gdb`
using "sudo" in order for the system to permit it to attach to a running process.
On a virtual machine this is not a big deal, but extreme caution should be used when
considering doing debugging under elevated privilege on a "bare metal" machine
where you might have stored important data that you can't afford to lose.

### Testing

The testing protocol to be used during grading has not been completely determined
as of the time of writing this document.  However, the primary objective of the
testing will be to try to determine if your implementations of the main process
and the engine process are capable of supporting reliable play of the game.
Because there are only two functions specified, there is a limited amount of
unit testing that we can do.  As far as "blackbox testing" is concerned, you
should expect that we will test your overall implementation by using the
`versus` driver to have it play against another program, so you will want to make
sure that functionality works.  We will likely attempt to stress test your signal
handling by setting `avgtime` to 0 so that moves are made "immediately", so you
will want to make sure that your program honors the time limitations set
by `-a`.  If your program takes too long to respond in certain situations,
that could be grounds for failing a test case.
We will likely attempt to test your implementation of
the main process separately from your implementation of the engine process by
using a stub for the `engine` function.  We may also choose to test your
`engine` function separately by substituting a driver of our own for your
main program.  We may also use instrumented versions of `versus` and/or `xdisp`
to observe and measure the behavior of your program.
We might also choose to use `valgrind` to check your implementation for serious errors,
such as invalid memory accesses and use of uninitialized variables or structure
fields.
Memory leaks are not expected to be an issue with this program, since the use
of dynamic storage allocation will likely be very limited or nonexistent.


### Demo program

For assignments like this one, which have fairly long and involved specifications,
I have found it useful in the past to provide, when possible, a demo version of
what you are supposed to implement, so that many questions that you might have
about the specifications can be answered by trying the demo program.
I have included such a demo program as `demo/ccheck` in the basecode distribution.
There is also a second version, called `demo/ccheck_db`, which produces debugging
trace information to help you understand what it is doing.
Although for the most part I expect the demo program to behave in accordance with
the assignment specifications, there might be some discrepancies.
In case of inconsistencies between the specifications in this document and the
behavior of the demo program, the specifications in this document take precedence.

## Hand-in instructions
As usual, make sure your homework compiles before submitting.
Test it carefully to be sure that doesn't crash or exhibit "flaky" behavior
due to race conditions.
Use `valgrind` to check for memory errors and leaks.
You might want to look into the `valgrind` `--trace-children` and related options.

Submit your work using `git submit` as usual.
This homework's tag is: `hw4`.
