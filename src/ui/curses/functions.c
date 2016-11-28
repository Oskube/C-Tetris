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

static const char symbols[7] = "0#$&8@%";
/**
    \brief Renders the current state of the game
    \param src Pointer to the game instance
    \param wins Pointer to the windows used in rendering
    \param showghost Determines if the ghost is shown
*/
static void DrawMap(game* src, curses_game_windows* wins, bool showghost);

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
    return 0;
}

void DrawHiscoresBegin(UI_Functions* data) {
    clear();
}

int GetNewHiscoreName(UI_Functions* data, hiscore_list_entry* entry, unsigned maxlen, unsigned rank) {
    if (!data) return 0;
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

    return event_ready;
}

void DrawText(UI_Functions* data, unsigned x, unsigned y, text_color color, char* text) {
    if (!data) return;
    curses_data* cdata = (curses_data*)data->data;
    WINDOW* dst = cdata->win;
    if (has_colors() && color < 8) {
        attron(COLOR_PAIR(color));
        mvwprintw(dst, y, x, "%s", text);
        attroff(COLOR_PAIR(color));
    } else {
        mvwprintw(dst, y, x, "%s", text);
    }
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

void DrawTetromino(UI_Functions* data, unsigned topx, unsigned topy, tetromino* tetr) {
    if (tetr == NULL) return;

    // Block locations are relative to tetromino's center
    topy += 1;
    topx += 1;
    block** blocks = tetr->blocks;
    for (unsigned i=0; i<4; i++) {
        mvaddch(topy+blocks[i]->y, topx+blocks[i]->x, symbols[blocks[i]->symbol]);
    }
}

void BeginGameInfo(UI_Functions* data, unsigned* x, unsigned* y) {
    if (!data) return;

    curses_data* cdata = (curses_data*)data->data;
    curses_game_windows* wins = (curses_game_windows*)cdata->additional;
    WINDOW* dst = wins->info;
    getbegyx(dst, *y, *x); // get position
    *x += 2; // move off the border
    *y += 1;

    werase(dst); // make sure info window is clean.
    wborder(dst, 0, 0, 0, 0, 0, 0, 0, 0);
}
/**
    Static functions
**/

void DrawMap(game* src, curses_game_windows* wins, bool showghost) {
    if (!src || !wins || !wins->map) return;

    WINDOW* dst = wins->map;

    //  Take game area dimensions
    unsigned w = src->map.width;
    unsigned h = src->map.height;

    //  If game is paused don't draw blocks
    if (src->info.status & GAME_STATUS_PAUSE) {
        wclear(dst); // Clear game area
        mvwprintw(dst, h/2-1, 3, "PAUSED!");
        wborder(dst, 0, 0, 0, 0, 0, 0, 0, 0); // Draw erased borders back
        return;
    }

    //  First draw already set tetrominos
    block** mask = src->map.blockMask;
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
