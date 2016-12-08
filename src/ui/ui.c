#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "ui.h"
#include "curses/init.h"
#include "sdl/init.h"
#include "states/states.h"

static char* generalHelp =
"Usage: tetr [options]\n\
General Options:\n \
  --help, -h\t\t\tDisplay this information\n \
  --demo, -d <path>\t\tPlay given demo record\n \
  --showkeys <0|1>\t\tShow pressed keys during demo playback\n \
  --randomiser, -r <name>\tSet randomiser used. Where name is 7bag, tgm or random\n \
  --srand <seed>\t\tSet seed used by randomiser\n \
  --UI <UI>\t\t\tSet UI used, see below\n\n\
UIs:\n ";

int MainProgram(int argc, char** argv) {
    void* (*CurrentState)(UI_Functions*, void**);
    void** data = (void**)malloc(sizeof(void*));
    *data = NULL;

    //  Pointer to UI initialization function
    int (*UIInitFun)(UI_Functions*, int, char**);
    UIInitFun = NULL;
    //  Defines what UI is used
    UI_Functions ui = {NULL};

    CurrentState = StateGame; //  Set game state as default
    unsigned stateArgs = 0; // index of state arguments in argv
    state_game_data gameSettings = {.randomiser = RANDOMISER_TGM};
    state_demo_data demoSettings = {.path = NULL, .showKeys = true};

    //  Initialize random seed
    srand((unsigned)time(NULL));

    //  Process command line arguments
    for (int i=1; i<argc; i++) {
        bool invalidArgs = false;
        if (!strcmp(argv[i], "--demo") || !strcmp(argv[i], "-d")) {
            if (argc <= ++i) {
                invalidArgs = true;
            } else {
                stateArgs = i; //  Save the argument index of demo path
                CurrentState = StatePlayDemo; // Set state to PlayDemo
            }
        }
        else if (!strcmp(argv[i], "--showkeys")) {
            if (argc <= ++i) {
                invalidArgs = true;
            } else {
                if(atoi(argv[i]) > 0) demoSettings.showKeys = true;
                else demoSettings.showKeys = false;
            }
        }
        else if (!strcmp(argv[i], "--randomiser") || !strcmp(argv[i], "-r")) {
            if (argc <= ++i) {
                invalidArgs = true;
            } else if (!strcmp(argv[i], "7bag")) {
                gameSettings.randomiser = RANDOMISER_BAG;
            } else if (!strcmp(argv[i], "tgm")) {
                gameSettings.randomiser = RANDOMISER_TGM;
            } else if (!strcmp(argv[i], "random")) {
                gameSettings.randomiser = RANDOMISER_RANDOM;
            } else {
                invalidArgs = true;
            }
        }
        //  Select UI
        else if (!strcmp(argv[i], "--UI")) {
            if (argc <= ++i) {
                invalidArgs = true;
#ifndef _NO_SDL
            } else if (!strcmp(argv[i], "SDL")) {
                UIInitFun = UI_SDLInit;
#endif //_NO_SDL
            } else if (!strcmp(argv[i], "curses")) {
                UIInitFun = CursesInit;
            } else {
                invalidArgs = true;
            }
        } else if (!strcmp(argv[i], "--srand")) {
            if (argc <= ++i) {
                invalidArgs = true;
            } else {
                srand(atoi(argv[i]));
            }
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            invalidArgs = true;
        }

        //  In case of invalid arguments, print help and quit
        if (invalidArgs) {
            fprintf(stderr, "Check arguments!\n");
            printf("%s%s", generalHelp, CursesGetHelp());
#ifndef _NO_SDL
            printf("%s", UI_SDLGetHelp());
#endif //_NO_SDL
            CurrentState = NULL;
            break;
        }
    }

    //  Set state specific settings
    if (CurrentState == StateGame) {
        //  Copy game settings
        if (*data != NULL) free(*data);
        state_game_data* set = (state_game_data*)malloc(sizeof(state_game_data));
        *set = gameSettings;
        *data = (void*)set;
    } else if (CurrentState == StatePlayDemo) {
        //  Allocate memory and copy demo path
        size_t len = strlen(argv[stateArgs]);
        char* str = (char*)malloc(sizeof(char)*len+1);
        strcpy(str, argv[stateArgs]);

        //  Malloc settings struct
        state_demo_data* set = (state_demo_data*)malloc(sizeof(state_demo_data));
        set->path = str;
        set->showKeys = demoSettings.showKeys;
        *data = (void*)set;
    }

    //  Default UI is curses
    if (!UIInitFun) UIInitFun = CursesInit;
    //  If next state is valid, init UI
    if (CurrentState && UIInitFun(&ui, argc, argv) != 0) {
        fprintf(stderr, "ERROR: UI init failed!\n");
        CurrentState = NULL;
    }

    // Main loop
    while (CurrentState != NULL) {
        CurrentState = CurrentState(&ui, data);

        ui.UIMainLoopEnd(&ui);
    }

    //  Make sure state data is freed
    if (*data != NULL) free(*data);
    free(data);
    //  Run UI specific clean up code
    if (ui.UICleanup) ui.UICleanup(&ui);

    return 0;
}
