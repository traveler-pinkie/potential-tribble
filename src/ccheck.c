#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "ccheck.h"
#include "debug.h"

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
    Player enginer_Player = -1;
    static int no_display = 1;
    static int tournament_mode = 1;


    while((option = getopt(argc, argv, "wbrvdta:i:o:")) != 1){
        switch(option){
            case 'w' :
                enginer_Player = X;
                break;
            case 'b' :
                enginer_Player = O;
                break;
            case 'r' :
                randomized = 5;
                break;
            case 'v' :
                verbose = 5;
                break;
            case 'd' :
                no_display = 0;
                break;
            case 't' :
                tournament_mode = 0;
                break;
            case 'a' :
                printf("Option -a\n", optarg);
                break;
            case 'i' :
                printf("Option -i\n",optarg);
                break;
            case 'o' :
                printf("Option -o\n", optarg);
                break;
            case ':': // Missing argument for an option
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                break;
            case '?': // Unknown option
                fprintf(stderr, "Unknown option: -%c.\n", optopt);
                break;
            default:
                break;
        }
    }

    // TO BE IMPLEMENTED
    fprintf(stderr, "The game will not work until you implement this function.\n");
    abort();
}
