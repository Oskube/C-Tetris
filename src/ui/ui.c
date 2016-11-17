#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "curses/init.h"
#include "sdl/init.h"
#include "states/states.h"

int MainProgram(int argc, char** argv) {
    void* (*CurrentState)(UI_Functions*, void**);
    void** data = (void**)malloc(sizeof(void*));
    *data = NULL;

    //  Pointer to UI initialization function
    int (*UIInitFun)(UI_Functions*, int, char**);
    UIInitFun = NULL;
    //  Defines what UI is used
    UI_Functions ui = {NULL};

    CurrentState = NULL;
    state_game_data gameSettings = {.randomiser = RANDOMISER_TGM};

    //  Game state is default
    if (!CurrentState) {
        CurrentState = StateGame;
    }

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
                fprintf(stderr, "Check arguments:\n--randomiser <name>\tWhere name is 7bag, tgm or random\n");
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
        //  Select SDL UI
        else if (!strcmp(argv[i], "--SDL") && !ui.UIGameInit) {
            UIInitFun = UI_SDLInit;
        }
    }

    if (CurrentState == StateGame) {
        //  Copy game settings
        if (*data != NULL) free(*data);
        state_game_data* set = (state_game_data*)malloc(sizeof(state_game_data));
        *set = gameSettings;
        *data = (void*)set;
    }

    //  Default UI is curses
    if (!UIInitFun) UIInitFun = CursesInit;
    if (UIInitFun(&ui, argc, argv) != 0) {
        fprintf(stderr, "ERROR: UI init failed!\n");
        CurrentState = NULL;
    }

    while (CurrentState != NULL) {
        CurrentState = CurrentState(&ui, data);

        ui.UIMainLoopEnd(&ui);
    }

    if (*data != NULL) free(*data);
    free(data);
    if (ui.UICleanup) ui.UICleanup(&ui);

    return 0;
}
