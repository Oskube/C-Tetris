#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* usleep() */

#include "curses_main.h"
#include "states.h"

int MainCurses() {
    WINDOW* win = initscr();

    if (!win) {
        fprintf(stderr, "CURSES: Can't initialize screen. Returning\n");
        return -1;
    }

    void* (*CurrentState)(WINDOW*, void**);
    void** data = (void**)malloc(sizeof(void*));
    *data = NULL;
    CurrentState = StateGame;

    while (CurrentState != NULL) {
        CurrentState = CurrentState(win, data);

        touchwin(win); /* Throw away all optimization info */
        wrefresh(win); /* Update terminal */
        usleep(1000);
    }

    free(data);
    endwin();
    return 0;
}
