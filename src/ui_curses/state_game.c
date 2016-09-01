#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> /* tolower() */

#include "states.h"
#include "game_functions.h"
#include "os_dependent.h"
#include "../core/hiscore.h"

//  Static fsm functions
static int StateInit(WINDOW* win, void** data);
static void StateCleanUp();

//  Static vars used by this state
static bool is_running = false;
static ncurse_game_windows windows = {.map = NULL, .info = NULL};
static game* gme = NULL;

//  State code
void* StateGame(WINDOW* win, void** data) {

    //  State init
    if (!is_running) {
        if (StateInit(win, data) != 0) return NULL;
        is_running = true;
    }
    //  State code
    //  Read and process user input
    int input = getch();
    switch (tolower(input)) {
        case 'w': ProcessInput(gme, INPUT_ROTATE); break;
        case 'a': ProcessInput(gme, INPUT_LEFT); break;
        case 's': ProcessInput(gme, INPUT_DOWN); break;
        case 'd': ProcessInput(gme, INPUT_RIGHT); break;
        case ' ': ProcessInput(gme, INPUT_SET); break;
        case 'q': is_running = false; break;
        default: break;
    }

    UpdateAndRender(gme, &windows);

    //  If quit requested
    if (!is_running) {
        *data = malloc(sizeof(hiscore_list_entry));
        if (!*data) return NULL;

        //  Fill entry struct
        hiscore_list_entry* e = (hiscore_list_entry*)*data;
        e->score = gme->info.score;
        e->rows = gme->info.rows;
        e->lvl = gme->info.level;
        e->time = GetGameTime(gme);

        StateCleanUp();
        return StateHiscores;
    }

    return StateGame;   //  Continue with current state
}

int StateInit(WINDOW* win, void** data) {
    if (!data) return -1;

    if (GameWindowsInit(win, &windows)) {
        return -2;
    }

    gme = Initialize(MAP_WIDTH, MAP_HEIGHT+2, RANDOMISER_TGM, GetTime);
    if (!gme) {
        fprintf(stderr, "CORE: Couldn't initialize game");
        GameWindowsFree(&windows);
        return -3;
    }
    return 0;
}

/**
    \brief State cleanup
*/
void StateCleanUp() {
    FreeGame(gme);
    gme = NULL;

    //  Free sub windows
    GameWindowsFree(&windows);
}
