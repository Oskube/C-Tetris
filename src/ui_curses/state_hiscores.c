#include <stdlib.h>
#include <string.h> /* strncpy */
#include <time.h> /* time() */
#include <ctype.h> /* tolower() */
#include <stdbool.h>

#include "states.h"

static int StateInit(UI_Functions* funs, void** data);
static void CleanUp();

static bool is_running = false;
static hiscore_list_entry scoreTable[HISCORE_LENGTH] = {0};
static hiscore_list_entry* entry = NULL;
static unsigned rank = HISCORE_LENGTH;
static char path_hiscore[256] = {0};

void* StateHiscores(UI_Functions* funs, void** data) {
    if (!is_running) {
        if (StateInit(funs, data) != 0) return NULL;
        is_running = true;
    }

    void* (*nextState)(UI_Functions*, void**);
    nextState = StateHiscores;

    if (entry != NULL) {
        funs->UIHiscoreGetName(funs, entry, 15, rank+1);
        AddScoreToList(scoreTable, HISCORE_LENGTH, entry);
        SaveHiScores(path_hiscore, scoreTable, HISCORE_LENGTH);
        free(entry);
        entry = NULL;

        funs->UIHiscoreRender(funs, scoreTable, HISCORE_LENGTH);
    }

    int input = funs->UIGetInput();
    switch (tolower(input)) {
        case 'q': {
            is_running = false;
            nextState = NULL;
        } break;
        case 'r': {
            is_running = false;
            nextState = StateGame;
        }
    }


    if (!is_running) CleanUp();
    return nextState;
}

int StateInit(UI_Functions* funs, void** data) {
    if (!data || !funs) return -1;

    //  Read high scores
    int len = funs->UIGetExePath(path_hiscore, 256);
    if (len < 0) return -2;
    strncpy(path_hiscore+len, HISCORE_FILE, 256-len);
    ReadHiScores(path_hiscore, scoreTable, HISCORE_LENGTH);

    entry = (hiscore_list_entry*)*data;
    rank = HISCORE_LENGTH;
    if (entry != NULL) {
        if(entry->score > 0) {
            rank = GetRanking(scoreTable, HISCORE_LENGTH, entry);
        }
        if (rank < HISCORE_LENGTH) {
            // Set date for entry
            entry->date = (unsigned)time(NULL);
        } else {
            free(entry);
            entry = NULL;
        }
        *data = NULL;
    }
    funs->UIHiscoreRender(funs, scoreTable, HISCORE_LENGTH);
    return 0;
}

void CleanUp() {
    is_running = false;
}
