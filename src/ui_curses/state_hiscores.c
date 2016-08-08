#include <stdlib.h>
#include <string.h> /* strncpy */
#include <time.h> /* time() */
#include <ctype.h> /* tolower() */

#include "states.h"
#include "os_dependent.h"
#include "../core/hiscore.h"


static void DrawHiscores(WINDOW* dst, unsigned topy, unsigned topx, hiscore_list_entry* list, unsigned len);

static int StateInit(WINDOW* w, void** data);
static void CleanUp();

static bool is_running = false;
static WINDOW* win = NULL;
static hiscore_list_entry scoreTable[HISCORE_LENGTH] = {0};
static hiscore_list_entry* entry = NULL;
static char path_hiscore[256] = {0};

void* StateHiscores(WINDOW* win, void** data) {
    if (!is_running) {
        if (StateInit(win, data) != 0) return NULL;
        is_running = true;
    }

    void* (*nextState)(WINDOW*, void**);
    nextState = StateHiscores;
    // mvprintw(0,0, "%s", path_hiscore); // Prints hiscore file path

    if (entry != NULL) {
        //  Get name and write it to file
        mvgetnstr(16, getmaxx(win)/2-5, entry->name, 15);
        AddScoreToList(scoreTable, HISCORE_LENGTH, entry);
        SaveHiScores(path_hiscore, scoreTable, HISCORE_LENGTH);
        free(entry);
        entry = NULL;

        nodelay(win, TRUE);
        noecho();
        curs_set(0);
        clear();
        DrawHiscores(win, 1, 5, scoreTable, HISCORE_LENGTH);
    }

    int input = getch();
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

int StateInit(WINDOW* w, void** data) {
    if (!data || !w) return -1;
    win = w;

    clear();
    //  Read high scores
    int len = GetExecutablePath(path_hiscore, 256);
    if (len < 0) return -2;
    strncpy(path_hiscore+len, HISCORE_FILE, 256-len);
    ReadHiScores(path_hiscore, scoreTable, HISCORE_LENGTH);

    entry = (hiscore_list_entry*)*data;
    if (entry != NULL) {
        unsigned rank = HISCORE_LENGTH;
        if(entry->score > 0) {
            rank = GetRanking(scoreTable, HISCORE_LENGTH, entry);
        }
        if (rank < HISCORE_LENGTH) {
            mvwprintw(w, 15, getmaxx(win)/2-12, "NEW HISCORE, %d. PLACE.", rank+1);

            // Set date for entry
            entry->date = (unsigned)time(NULL);

            nodelay(win, FALSE);
            echo();
            curs_set(1);
        } else {
            free(entry);
            entry = NULL;
        }
        *data = NULL;
    }
    DrawHiscores(win, 1, 5, scoreTable, HISCORE_LENGTH);
    return 0;
}

void CleanUp() {
    is_running = false;
}

void DrawHiscores(WINDOW* dst, unsigned topy, unsigned topx, hiscore_list_entry* list, unsigned len) {
    mvwprintw(dst, topy++, getmaxx(win)/2-10, "TOP %d SCORES", len);
    mvwprintw(dst, topy, topx, "#");
    mvwprintw(dst, topy, topx+3, "NAME");
    mvwprintw(dst, topy, topx+20, "SCORE");
    mvwprintw(dst, topy, topx+30, "LVL");
    mvwprintw(dst, topy, topx+35, "TIME");
    mvwprintw(dst, topy++, topx+50, "DATE");
    for (unsigned i=0; i<len; i++, topy++) {
        time_t date = (time_t)list[i].date;
        unsigned sec    = list[i].time/1000;
        unsigned min    = sec%3600/60;
        unsigned tenth  = list[i].time/100%10;
        sec %= 60;

        mvwprintw(dst, topy, topx, "%d", i+1);
        mvwprintw(dst, topy, topx+3, "%s", list[i].name);
        mvwprintw(dst, topy, topx+20, "%d", list[i].score);
        mvwprintw(dst, topy, topx+30, "%d", list[i].lvl);
        mvwprintw(dst, topy, topx+35, "%d:%02d.%d", min, sec, tenth);
        mvwprintw(dst, topy, topx+50, "%s", ctime(&date));
    }
}
