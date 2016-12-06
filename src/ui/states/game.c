#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> /* tolower() */
#include <time.h> /* strftime(), localtime(), time()*/
#include <string.h> /* strcpy() */
#include <stdbool.h>

#include "states.h"
#include "common.h"

//  Static fsm functions
static int StateInit(UI_Functions* funs, void** data);
static void StateCleanUp(UI_Functions* funs);

static char* GenerateDemoName(UI_Functions* funs);
static void ShowHelp(UI_Functions* funs, unsigned x, unsigned y, bool showSave);

//  Static vars used by this state
static bool is_running = false;
static game* gme = NULL;
static bool alreadySaved = false;
static state_game_data settings = {0}; /* Game settings, stays same until changed */

static char textDemo[128] = {0}; //  Demo saved text

//  State code
void* StateGame(UI_Functions* funs, void** data) {
    //  State init
    if (!is_running) {
        if (StateInit(funs, data) != 0) return NULL;
        is_running = true;
    }

    //  State code
    //  Read and process user input
    unsigned icount = funs->UIGetInput(funs); // Fill input array
    for (unsigned iii = 0; iii < icount; iii++) { // Process all inputs
        switch (tolower(funs->inputs[iii])) {
            case 'w': GameProcessInput(gme, INPUT_ROTATE); break;
            case 'a': GameProcessInput(gme, INPUT_LEFT); break;
            case 's': GameProcessInput(gme, INPUT_DOWN); break;
            case 'd': GameProcessInput(gme, INPUT_RIGHT); break;
            case ' ': GameProcessInput(gme, INPUT_SET); break;
            case 'q': is_running = false; break;
            case 'p': GameTogglePause(gme); break;
            case 'r': if ((gme->info.status & GAME_STATUS_END) && !alreadySaved) {
                alreadySaved = true;

                char* name = GenerateDemoName(funs);
                if (!name) break;

                snprintf(textDemo, 128, "Demo saved: %s", name);

                DemoSave(gme->demorecord, name);
                free(name);
            } break;
            default: break;
        }
    }

    //  Print msg if is demo saved
    if (textDemo[0] != '\0') funs->UITextRender(funs, 0, 0, color_red, textDemo);

    GameUpdate(gme);

    unsigned x, y;
    ShowGameInfo(funs, gme, true, &x, &y);

    ShowHelp(funs, x+18, y+7, gme->info.status & GAME_STATUS_END);
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
        e->time = GameGetTime(gme);

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

    gme = GameInitialize(MAP_WIDTH, MAP_HEIGHT+2, settings.randomiser, funs->UIGetMillis);
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
    GameFree(gme);
    gme = NULL;

    //  Free sub windows
    funs->UIGameCleanup(funs);
}

char* GenerateDemoName(UI_Functions* funs) {
    static const unsigned strLen = 64;
    char* ret = (char*)calloc(strLen, sizeof(char));
    if (ret) {
        int len = funs->UIGetExePath(funs, ret, strLen);
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

void ShowHelp(UI_Functions* funs, unsigned x, unsigned y, bool showSave) {
    funs->UITextRender(funs, x, y++, color_white, "Controls:");
    funs->UITextRender(funs, x, y++, color_green, "LEFT, RIGHT, DOWN - Move tetromino");
    funs->UITextRender(funs, x, y++, color_green, "SPACE             - Set tetromino");
    funs->UITextRender(funs, x, y++, color_green, "P                 - Pause");
    funs->UITextRender(funs, x, y++, color_green, "Q                 - QUIT");

    if (showSave)
        funs->UITextRender(funs, x, y, color_red, "R                 - Save demo");
}
