#include <stdio.h> /* snprintf() */
#include <stdlib.h> /* free() */
#include <string.h> /* strncpy */
#include <time.h> /* time() */
#include <ctype.h> /* tolower() */
#include <stdbool.h>

#include "states.h"

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

    int icount = 0;
    if (entry != NULL) {
        DrawHiscores(funs, scoreTable, HISCORE_LENGTH);
        icount = funs->UIHiscoreGetName(funs, entry, 15, rank+1);
        if (icount > 0 && funs->inputs[0] == event_ready) {
            AddScoreToList(scoreTable, HISCORE_LENGTH, entry);
            SaveHiScores(path_hiscore, scoreTable, HISCORE_LENGTH);
            free(entry);
            entry = NULL;
            DrawHiscores(funs, scoreTable, HISCORE_LENGTH);
        }
    } else {
        icount = funs->UIGetInput(funs);  // Fill input array
    }

    for (unsigned iii = 0; iii < icount; iii++) { // Process all inputs
        switch (tolower(funs->inputs[iii])) {
            case 'q': {
                is_running = false;
                nextState = NULL;
            } break;
            case 'r': {
                is_running = false;
                nextState = StateGame;
            }
            case event_req_refresh: DrawHiscores(funs, scoreTable, HISCORE_LENGTH); break;
            default: break;
        }
    }

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
    funs->UITextRender(funs, 34, topy++, color_default, temp);
    funs->UITextRender(funs, topx,topy, color_blue, "#");
    funs->UITextRender(funs, topx+3,topy, color_blue, "NAME");
    funs->UITextRender(funs, topx+20,topy, color_blue, "SCORE");
    funs->UITextRender(funs, topx+30,topy, color_blue, "LINES");
    funs->UITextRender(funs, topx+37,topy, color_blue, "LVL");
    funs->UITextRender(funs, topx+41,topy, color_blue, "TIME");
    funs->UITextRender(funs, topx+50,topy++, color_blue, "DATE");

    for (unsigned i=0; i<len; i++, topy++) {
        time_t date = (time_t)list[i].date;
        unsigned sec    = list[i].time/1000;
        unsigned min    = sec%3600/60;
        unsigned tenth  = list[i].time/100%10;
        sec %= 60;

        snprintf(temp, 256, "%d", i+1);
        funs->UITextRender(funs, topx, topy, color_red, temp);

        funs->UITextRender(funs, topx+3, topy, color_green, list[i].name);
        snprintf(temp, 256, "%d", list[i].score);
        funs->UITextRender(funs, topx+20, topy, color_default, temp);
        snprintf(temp, 256, "%d", list[i].rows);
        funs->UITextRender(funs, topx+30, topy, color_magenta, temp);
        snprintf(temp, 256, "%d", list[i].lvl);
        funs->UITextRender(funs, topx+37, topy, color_magenta, temp);
        snprintf(temp, 256, "%d:%02d.%d", min, sec, tenth);
        funs->UITextRender(funs, topx+41, topy, color_magenta, temp);
        funs->UITextRender(funs, topx+50, topy, color_yellow, ctime(&date));
    }
}
