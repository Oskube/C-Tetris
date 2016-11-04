#include <stdlib.h>
#include <string.h> /* strncpy */
#include <time.h> /* time() */
#include <ctype.h> /* tolower() */
#include <stdbool.h>

#include "states.h"

#define COLOR_RED 0xffff0000

static int StateInit(UI_Functions* funs, void** data);
static void CleanUp();

static void DrawHiscores(UI_Functions* funs, hiscore_list_entry* list, unsigned len);

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
    }

    int input = funs->UIGetInput(funs);
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

    DrawHiscores(funs, scoreTable, HISCORE_LENGTH);

    if (!is_running) CleanUp();
    return nextState;
}

int StateInit(UI_Functions* funs, void** data) {
    if (!data || !funs) return -1;

    //  Read high scores
    int len = funs->UIGetExePath(funs, path_hiscore, 256);
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
    DrawHiscores(funs, scoreTable, HISCORE_LENGTH);
    return 0;
}

void CleanUp() {
    is_running = false;
}

void DrawHiscores(UI_Functions* funs, hiscore_list_entry* list, unsigned len) {
    if (!list) return;

    funs->UIHiscoreRenderBegin(funs);

    char temp[256] = {0};
    unsigned topy = 1;
    unsigned topx = 3;

    snprintf(temp, 256, "TOP %d SCORES", len);
    funs->UITextRender(funs, 34, topy++, COLOR_RED, temp);
    funs->UITextRender(funs, topx,topy, COLOR_RED, "#");
    funs->UITextRender(funs, topx+3,topy, COLOR_RED, "NAME");
    funs->UITextRender(funs, topx+20,topy, COLOR_RED, "SCORE");
    funs->UITextRender(funs, topx+30,topy, COLOR_RED, "LINES");
    funs->UITextRender(funs, topx+37,topy, COLOR_RED, "LVL");
    funs->UITextRender(funs, topx+41,topy, COLOR_RED, "TIME");
    funs->UITextRender(funs, topx+50,topy++, COLOR_RED, "DATE");

    for (unsigned i=0; i<len; i++, topy++) {
        time_t date = (time_t)list[i].date;
        unsigned sec    = list[i].time/1000;
        unsigned min    = sec%3600/60;
        unsigned tenth  = list[i].time/100%10;
        sec %= 60;

        snprintf(temp, 256, "%d", i+1);
        funs->UITextRender(funs, topx, topy, COLOR_RED, temp);

        funs->UITextRender(funs, topx+3, topy, COLOR_RED, list[i].name);
        snprintf(temp, 256, "%d", list[i].score);
        funs->UITextRender(funs, topx+20, topy, COLOR_RED, temp);
        snprintf(temp, 256, "%d", list[i].rows);
        funs->UITextRender(funs, topx+30, topy, COLOR_RED, temp);
        snprintf(temp, 256, "%d", list[i].lvl);
        funs->UITextRender(funs, topx+37, topy, COLOR_RED, temp);
        snprintf(temp, 256, "%d:%02d.%d", min, sec, tenth);
        funs->UITextRender(funs, topx+41, topy, COLOR_RED, temp);
        funs->UITextRender(funs, topx+50, topy, COLOR_RED, ctime(&date));
    }
}
