#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* usleep() */

#include "curses_main.h"
#include "states.h"

int MainCurses(int argc, char** argv) {
    WINDOW* win = initscr();

    if (!win) {
        fprintf(stderr, "CURSES: Can't initialize screen. Returning\n");
        return -1;
    }

    void* (*CurrentState)(WINDOW*, void**);
    void** data = (void**)malloc(sizeof(void*));
    *data = NULL;

    CurrentState = StateGame;
    //  Process arguments
    for (unsigned i=1; i<argc; i++) {
        if (strcmp(argv[i], "--demo") == 0) {
            if (argc <= ++i || *data != NULL) {
                fprintf(stderr, "Check arguments\n");
                CurrentState = NULL;
                break;
            }

            //  Allocate memory and copy path
            size_t len = strlen(argv[i]);
            *data = malloc(sizeof(char)*len+1);
            strcpy((char*)*data, argv[i]);
            CurrentState = StatePlayDemo; // Set state to PlayDemo
        }
    }


    while (CurrentState != NULL) {
        CurrentState = CurrentState(win, data);

        touchwin(win); /* Throw away all optimization info */
        wrefresh(win); /* Update terminal */
        usleep(1000);
    }

    if (*data != NULL) free(*data);
    free(data);
    endwin();
    return 0;
}
