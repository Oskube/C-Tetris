#include <stdlib.h> /*malloc*/
#include <time.h> /*used in DrawHiscores()*/
#include <curses.h>

#include "../os/os.h"
#include "functions.h"

/**
    \brief Struct for windows used by game functions
*/
typedef struct {
    WINDOW* map; /**< Window for game area or map*/
    WINDOW* info; /**< Window for game info */
} curses_game_windows;

//  Renders tetromino
static void DrawTetromino(tetromino* tetr, WINDOW* dst, unsigned topy, unsigned topx);
//  Renders statistics of pieces spawned
static void DrawStats(game* src, WINDOW* dst, unsigned topy, unsigned topx);

/**
    \brief Renders the current state of the game
    \param src Pointer to the game instance
    \param wins Pointer to the windows used in rendering
    \param showghost Determines if the ghost is shown
*/
static void DrawMap(game* src, curses_game_windows* wins, bool showghost);

/**
    \brief Renders game info

    Shows current score and count of spawned pieces.
    \param src Pointer to the game instance
    \param wins Pointer to the windows used in rendering
*/
static void DrawInfo(game* src, curses_game_windows* wins);

int GameWindowsInit(UI_Functions* data) {
    if (!data) return -1;
    curses_data* cdata = (curses_data*)data->data;
    WINDOW* win = cdata->win;
    if (!win) return -2;

    clear();
    //  Get window size
    unsigned winCols = 0;
    unsigned winRows = 0;
    getmaxyx(win, winRows, winCols);

    cbreak();   // Disable line buffering
    noecho();   // Don't show user input
    nodelay(win, TRUE); // Make getch non-blocking
    nonl();     // We can detect return key
    intrflush(stdscr, FALSE); // Disable tty buffer flushing
    keypad(stdscr, TRUE); // Enable function keys etc.
    curs_set(0);    //  Hide console cursor

    //  Initialize map window
    WINDOW* map = subwin(win, MAP_HEIGHT+2, MAP_WIDTH+2, 1, 2);
    wborder(map, 0, 0, 0, 0, 0, 0, 0, 0);

    //  Sub window for stats and other info
    WINDOW* info = subwin(win, winRows-2, winCols-MAP_WIDTH-8, 1, MAP_WIDTH+6);

    if (!info || !map) {
        fprintf(stderr, "CURSES: Subwindow creation failed.");
        return -2;
    }

    curses_game_windows* gwins = (curses_game_windows*)malloc(sizeof(curses_game_windows));
    if (!gwins) return -3;
    gwins->map = map;
    gwins->info = info;
    cdata->additional = (void*)gwins;

    return 0;
}

void GameWindowsFree(UI_Functions* data) {
    if (!data) return;
    curses_data* cdata = (curses_data*)data->data;

    curses_game_windows* windows = (curses_game_windows*)cdata->additional;
    if (windows == NULL) return;

    //  Free map
    delwin(windows->map);
    windows->map = NULL;
    //  Free info
    delwin(windows->info);
    windows->info = NULL;

    free(cdata->additional);
    cdata->additional = NULL;
}

int GameRender(UI_Functions* data, game* gme) {
    if (!data) return -1;
    curses_data* cdata = (curses_data*)data->data;

    curses_game_windows* windows = (curses_game_windows*)cdata->additional;
    if (windows == NULL) return -2;

    DrawMap(gme, windows, true);
    DrawInfo(gme, windows);
    return 0;
}

void DrawHiscores(UI_Functions* data, hiscore_list_entry* list, unsigned len) {
    if (!data) return;
    curses_data* cdata = (curses_data*)data->data;
    WINDOW* dst = cdata->win;

    clear();
    unsigned topy = 1;
    unsigned topx = 3;
    mvwprintw(dst, topy++, getmaxx(dst)/2-10, "TOP %d SCORES", len);
    mvwprintw(dst, topy, topx, "#");
    mvwprintw(dst, topy, topx+3, "NAME");
    mvwprintw(dst, topy, topx+20, "SCORE");
    mvwprintw(dst, topy, topx+30, "LINES");
    mvwprintw(dst, topy, topx+37, "LVL");
    mvwprintw(dst, topy, topx+41, "TIME");
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
        mvwprintw(dst, topy, topx+30, "%d", list[i].rows);
        mvwprintw(dst, topy, topx+37, "%d", list[i].lvl);
        mvwprintw(dst, topy, topx+41, "%d:%02d.%d", min, sec, tenth);
        mvwprintw(dst, topy, topx+50, "%s", ctime(&date));
    }
}
void GetNewHiscoreName(UI_Functions* data, hiscore_list_entry* entry, unsigned maxlen, unsigned rank) {
    if (!data) return;
    curses_data* cdata = (curses_data*)data->data;
    WINDOW* dst = cdata->win;

    //  Change modes and show cursor
    nodelay(dst, FALSE);
    echo();
    curs_set(1);

    //  Print message and ask name
    mvwprintw(dst, 15, getmaxx(dst)/2-12, "NEW HISCORE, %d. PLACE.", rank);
    mvgetnstr(16, getmaxx(dst)/2-5, entry->name, maxlen);

    //  Revert modes
    nodelay(dst, TRUE);
    noecho();
    curs_set(0);
    clear();
}
void DrawText(UI_Functions* data, unsigned x, unsigned y, char* text) {
    if (!data) return;
    curses_data* cdata = (curses_data*)data->data;
    WINDOW* dst = cdata->win;

    mvwprintw(dst, x, y, "%s", text);
}

int ReadKey(UI_Functions* funs) {
    return getch();
};

void LoopEnd(UI_Functions* data) {
    if (!data) return;
    curses_data* cdata = (curses_data*)data->data;
    WINDOW* win = cdata->win;

    touchwin(win); /* Throw away all optimization info */
    wrefresh(win); /* Update terminal */
    SleepMs(2);
}

int GetExePath(UI_Functions* data, char* buf, unsigned len) {
    return GetExecutablePath(buf, len);
}

/**
    Static functions
**/

void DrawMap(game* src, curses_game_windows* wins, bool showghost) {
    if (!src || !wins || !wins->map) return;

    WINDOW* dst = wins->map;

    //  First draw already set tetrominos
    unsigned w = src->map.width;
    unsigned h = src->map.height;
    block** mask = src->map.blockMask;
    const char symbols[7] = "0#$&8@%";

    unsigned len = w*h;
    unsigned row = 1;
    wmove(dst, row, 1);
    //  Two top rows are hidden so start from 2*w
    for (unsigned i = w*2; i < len; i++) {
        if (mask[i]) {
            waddch(dst, symbols[mask[i]->symbol]);
        } else {
            waddch(dst, ' ');
        }
        if ((i+1)%w == 0) {
            wmove(dst, ++row, 1);
        }
    }

    //  Draw active tetromino
    if (src->active != NULL) {
        mask = src->active->blocks;

        int x[4],
            y[4];
        char sym = symbols[mask[0]->symbol];
        for (unsigned i = 0; i < 4; i++) {
            y[i] = mask[i]->y+1;
            x[i] = mask[i]->x+1;
        }
        if (showghost) {
            for (unsigned i=0; i < 4; i++) {
                unsigned cy = y[i]+src->info.ghostY;
                // Top 2 rows are hidden
                if (cy > 2) {
                    mvwaddch(dst, cy-2, x[i]+src->active->x, ':');
                }
            }
        }

        for (unsigned i=0; i < 4; i++) {
            unsigned cy = y[i]+src->active->y;
            // Top 2 rows are hidden
            if (cy > 2) {
                mvwaddch(dst, cy-2, x[i]+src->active->x, sym);
            }
        }
    }
}

void DrawInfo(game* src, curses_game_windows* wins) {
    if (!src || !wins || !wins->info) return;

    WINDOW* dst = wins->info;

    game_info* s = &src->info;
    werase(dst);
    wborder(dst, 0, 0, 0, 0, 0, 0, 0, 0);

    mvwprintw(dst, 1, 2, "Level: %d (%d)", s->level, s->rowsToNextLevel);
    mvwprintw(dst, 2, 2, "Lines: %d", s->rows);
    mvwprintw(dst, 3, 2, "Score: %d", s->score);
    mvwprintw(dst, 4, 2, "Next:", s->score);
    DrawTetromino(src->info.next, dst, 4, 8);
    DrawStats(src, dst, 8, 2);
}

void DrawTetromino(tetromino* tetr, WINDOW* dst, unsigned topy, unsigned topx) {
    if (tetr == NULL) return;

    const char symbols[7] = "0#$&8@%";
    // Block locations are relative to tetromino's center
    topy += 1;
    topx += 1;
    block** blocks = tetr->blocks;
    for (unsigned i=0; i<4; i++) {
        mvwaddch(dst, topy+blocks[i]->y, topx+blocks[i]->x, symbols[blocks[i]->symbol]);
    }
}

void DrawStats(game* src, WINDOW* dst, unsigned topy, unsigned topx) {
    unsigned total = 0;
    static char* minos = "OITLJSZ";
    mvwprintw(dst, topy++, topx, "Statistics:");
    for (unsigned i=0; i < 7; topy++, i++) {
        mvwprintw(dst, topy, topx, "%c: %d", minos[i], src->info.countTetromino[i]);
        total += src->info.countTetromino[i];
    }

    mvwprintw(dst, topy, topx, "Total: %d", total);
}
