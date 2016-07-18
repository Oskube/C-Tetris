#include <curses.h>
#include <stdio.h>

#include "UI_curses.h"
#include "game.h"

#define MAP_WIDTH  10
#define MAP_HEIGHT 20

static void DrawMap(game* src, WINDOW* dst, bool showghost);
static void DrawInfo(game* src, WINDOW* dst);
static void DrawNextTetromino(tetromino* tetr, WINDOW* dst, unsigned topy, unsigned topx);

int MainCurses() {
    WINDOW* win = initscr();

    if (!win) {
        fprintf(stderr, "CURSES: Can't initialize screen. Returning\n");
        return -1;
    }

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
    WINDOW* stats = subwin(win, winRows-2, winCols-MAP_WIDTH-8, 1, MAP_WIDTH+6);

    if (!stats || !map) {
        fprintf(stderr, "CURSES: Subwindow creation failed.");
        endwin();
        return -2;
    }

    game* gme = Initialize(MAP_WIDTH, MAP_HEIGHT, RANDOMISER_TGM);
    if (!gme) {
        fprintf(stderr, "CORE: Couldn't initialize game");
        delwin(map);
        delwin(stats);
        endwin();
        return -3;
    }

    bool quit = false;
    while (!quit) {
        //  Read and process user input
        int input = getch();
        switch (input) {
            case 'w': ProcessInput(gme, INPUT_ROTATE); break;
            case 'a': ProcessInput(gme, INPUT_LEFT); break;
            case 's': ProcessInput(gme, INPUT_DOWN); break;
            case 'd': ProcessInput(gme, INPUT_RIGHT); break;
            case ' ': ProcessInput(gme, INPUT_SET); break;
            case 'q': quit = true; break;
            default: break;
        }

        //  Update game logic
        Update(gme);

        //  TODO: Refresh only when something has changed
        //  Update UI
        DrawMap(gme, map, true);
        DrawInfo(gme, stats);

        touchwin(win); /* Throw away all optimization info */
        wrefresh(win); /* Update terminal */
    }
    //  Clean up
    FreeGame(gme);

    delwin(stats);
    delwin(map);
    endwin();
    return 0;
}

void DrawMap(game* src, WINDOW* dst, bool showghost) {
    if (!src) return;

    //  First draw already set tetrominos
    unsigned w = src->map.width;
    unsigned h = src->map.height;
    block** mask = src->map.blockMask;

    unsigned len = w*h;
    unsigned row = 1;
    wmove(dst, row, 1);
    for (unsigned i = 0; i < len; i++) {
        if (mask[i]) {
            waddch(dst, '#');
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
        for (unsigned i = 0; i < 4; i++) {
            y[i] = mask[i]->y+1;
            x[i] = mask[i]->x+1;
        }
        if (showghost) {
            mvwaddch(dst, y[0]+src->info.ghostY, x[0]+src->active->x, ':');
            mvwaddch(dst, y[1]+src->info.ghostY, x[1]+src->active->x, ':');
            mvwaddch(dst, y[2]+src->info.ghostY, x[2]+src->active->x, ':');
            mvwaddch(dst, y[3]+src->info.ghostY, x[3]+src->active->x, ':');
        }

        mvwaddch(dst, y[0]+src->active->y, x[0]+src->active->x, '#');
        mvwaddch(dst, y[1]+src->active->y, x[1]+src->active->x, '#');
        mvwaddch(dst, y[2]+src->active->y, x[2]+src->active->x, '#');
        mvwaddch(dst, y[3]+src->active->y, x[3]+src->active->x, '#');
    }
}

void DrawInfo(game* src, WINDOW* dst) {
    if (!src) return;

    game_info* s = &src->info;
    werase(dst);
    wborder(dst, 0, 0, 0, 0, 0, 0, 0, 0);

    mvwprintw(dst, 1, 2, "Level: %d (%d)", s->level, s->rowsToNextLevel);
    mvwprintw(dst, 2, 2, "Lines: %d", s->rows);
    mvwprintw(dst, 3, 2, "Score: %d", s->score);
    mvwprintw(dst, 4, 2, "Next:", s->score);
    DrawNextTetromino(src->info.next, dst, 4, 8);
}

void DrawNextTetromino(tetromino* tetr, WINDOW* dst, unsigned topy, unsigned topx) {
    if (tetr == NULL) return;

    // Block locations are relative to tetromino's center
    topy += 1;
    topx += 1;
    block** blocks = tetr->blocks;
    for (unsigned i=0; i<4; i++) {
        mvwaddch(dst, topy+blocks[i]->y, topx+blocks[i]->x, '#');
    }
}
