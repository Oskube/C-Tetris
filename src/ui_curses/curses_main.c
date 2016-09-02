#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* usleep() */

#include "curses_main.h"
#include "states.h"
#include "../core/game_randomisers.h"

int MainCurses(int argc, char** argv) {
    WINDOW* win = initscr();

    if (!win) {
        fprintf(stderr, "CURSES: Can't initialize screen. Returning\n");
        return -1;
    }

    void* (*CurrentState)(WINDOW*, void**);
    void** data = (void**)malloc(sizeof(void*));
    *data = NULL;

    CurrentState = NULL;
    state_game_data gameSettings = {.randomiser = RANDOMISER_TGM};
    //  Process arguments
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--demo")) {
            if (argc <= ++i || *data != NULL) {
                fprintf(stderr, "Check arguments\n--demo <path>\n");
                CurrentState = NULL;
                break;
            }

            //  Allocate memory and copy path
            size_t len = strlen(argv[i]);
            *data = malloc(sizeof(char)*len+1);
            strcpy((char*)*data, argv[i]);
            CurrentState = StatePlayDemo; // Set state to PlayDemo
        }
        else if (!strcmp(argv[i], "--randomiser")) {
            if (argc <= ++i) {
                fprintf(stderr, "Check arguments:\n--randomiser <name>\nWhere name is 7bag, tgm or random\n");
                CurrentState = NULL;
                break;
            } else if (!strcmp(argv[i], "7bag")) {
                gameSettings.randomiser = RANDOMISER_BAG;
            } else if (!strcmp(argv[i], "tgm")) {
                gameSettings.randomiser = RANDOMISER_TGM;
            } else if (!strcmp(argv[i], "random")) {
                gameSettings.randomiser = RANDOMISER_RANDOM;
            }
        }
    }

    //  Game state is default
    if (!CurrentState) {
        CurrentState = StateGame;
        //  Copy game settings
        if (*data != NULL) free(*data);
        state_game_data* set = (state_game_data*)malloc(sizeof(state_game_data));
        *set = gameSettings;
        *data = (void*)set;
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
