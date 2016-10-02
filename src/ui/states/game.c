#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> /* tolower() */
#include <time.h> /* strftime(), localtime(), time()*/
#include <string.h> /* strcpy() */
#include <stdbool.h>

#include "states.h"

//  Static fsm functions
static int StateInit(UI_Functions* funs, void** data);
static void StateCleanUp(UI_Functions* funs);
static char* GenerateDemoName(UI_Functions* funs);

//  Static vars used by this state
static bool is_running = false;
static game* gme = NULL;
static bool alreadySaved = false;
static state_game_data settings = {0}; /* Game settings, stays same until changed */

//  State code
void* StateGame(UI_Functions* funs, void** data) {
    //  State init
    if (!is_running) {
        if (StateInit(funs, data) != 0) return NULL;
        is_running = true;
    }

    //  State code
    //  Read and process user input
    int input = funs->UIGetInput();
    switch (tolower(input)) {
        case 'w': ProcessInput(gme, INPUT_ROTATE); break;
        case 'a': ProcessInput(gme, INPUT_LEFT); break;
        case 's': ProcessInput(gme, INPUT_DOWN); break;
        case 'd': ProcessInput(gme, INPUT_RIGHT); break;
        case ' ': ProcessInput(gme, INPUT_SET); break;
        case 'q': is_running = false; break;
        case 'p': if (gme->info.ended && !alreadySaved) {
            alreadySaved = true;

            char* name = GenerateDemoName(funs);
            if (!name) break;

            char text[128];
            snprintf(text, 128, "Demo saved: %s", name);
            funs->UITextRender(funs, 0, 0, text);
            DemoSave(gme->demorecord, name);
            free(name);
        } break;
        default: break;
    }

    Update(gme);
    funs->UIGameRender(funs, gme);

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

        StateCleanUp(funs);
        return StateHiscores;
    }

    return StateGame;   //  Continue with current state
}

int StateInit(UI_Functions* funs, void** data) {
    if (!data) return -1;

    if (funs->UIGameInit(funs)) {
        return -2;
    }

    //  Copy settings if any
    if (*data) {
        settings = *((state_game_data*)*data);
        free(*data);
        *data = NULL;
    }

    gme = Initialize(MAP_WIDTH, MAP_HEIGHT+2, settings.randomiser, funs->UIGetMillis);
    if (!gme) {
        fprintf(stderr, "CORE: Couldn't initialize game");
        funs->UIGameCleanup(funs);
        return -3;
    }

    alreadySaved = false;
    return 0;
}

/**
    \brief State cleanup
*/
void StateCleanUp(UI_Functions* funs) {
    FreeGame(gme);
    gme = NULL;

    //  Free sub windows
    funs->UIGameCleanup(funs);
}

char* GenerateDemoName(UI_Functions* funs) {
    static const unsigned strLen = 64;
    char* ret = (char*)calloc(strLen, sizeof(char));
    if (ret) {
        int len = funs->UIGetExePath(ret, strLen);
        if (len < 0) {
            free(ret);
            return NULL;
        } else if(strLen-len < 30) {
            // not enough space for filename -> allocated new and copy contents
            char* newret = (char*)calloc(len+sizeof(char)*30, sizeof(char));
            strcpy(newret, ret);
            free(ret);
            ret = newret;
        }
        time_t t = time(NULL);
        struct tm* tmp = localtime(&t);
        strftime(ret+len, strLen+30, "%Y%m%d-%H%M%S.demo", tmp);
    }
    return ret;
}