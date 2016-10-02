#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "curses/init.h"
#include "states/states.h"

int MainProgram(int argc, char** argv) {
    void* (*CurrentState)(UI_Functions*, void**);
    void** data = (void**)malloc(sizeof(void*));
    *data = NULL;

    // Use Curses as UI
    UI_Functions ui;
    if (CursesInit(&ui) != 0) {
        fprintf(stderr, "Curses Initialization failed");
        return -1;
    }

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
        CurrentState = CurrentState(&ui, data);

        ui.UIMainLoopEnd(&ui);
    }

    if (*data != NULL) free(*data);
    free(data);
    ui.UICleanup(&ui);

    return 0;
}