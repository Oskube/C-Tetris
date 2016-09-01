#include "game_functions.h"

//  Renders tetromino
static void DrawTetromino(tetromino* tetr, WINDOW* dst, unsigned topy, unsigned topx);
//  Renders statistics of pieces spawned
static void DrawStats(game* src, WINDOW* dst, unsigned topy, unsigned topx);


int GameWindowsInit(WINDOW* win, ncurse_game_windows* ret) {
    if (!win || !ret) return -1;
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
        endwin();
        return -2;
    }

    ret->map = map;
    ret->info = info;
    return 0;
}

void GameWindowsFree(ncurse_game_windows* windows) {
    if (windows == NULL) return;

    //  Free map
    delwin(windows->map);
    windows->map = NULL;
    //  Free info
    delwin(windows->info);
    windows->info = NULL;
}

int UpdateAndRender(game* gme, ncurse_game_windows* windows) {
    //  Update game logic
    int ret = Update(gme);

    //  TODO: Refresh only when something has changed
    //  Update UI
    DrawMap(gme, windows, true);
    DrawInfo(gme, windows);

    return ret;
}

void DrawMap(game* src, ncurse_game_windows* wins, bool showghost) {
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

void DrawInfo(game* src, ncurse_game_windows* wins) {
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

/**
    Static functions
**/

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
