#include <curses.h>
#include <stdio.h>
#include <unistd.h> /* usleep() */

#include "UI_curses.h"
#include "ncurse_states.h"

int MainCurses() {
    WINDOW* win = initscr();

    if (!win) {
        fprintf(stderr, "CURSES: Can't initialize screen. Returning\n");
        return -1;
    }

    void* (*CurrentState)(WINDOW*, void**);
    CurrentState = StateGame;
    while (CurrentState != NULL) {
        CurrentState = CurrentState(win, NULL);

        touchwin(win); /* Throw away all optimization info */
        wrefresh(win); /* Update terminal */
        usleep(1000);
    }

    endwin();
    return 0;
}
