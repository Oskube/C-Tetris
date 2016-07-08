#include <curses.h>
#include <stdio.h>

#include "UI_curses.h"
#include "game.h"

#define MAP_WIDTH  10
#define MAP_HEIGHT 20

static void DrawMap(game* src, WINDOW* dst);
static void DrawInfo(game* src, WINDOW* dst);

int MainCurses() {
    WINDOW* win = initscr();

    if (!win) {
        fprintf(stderr, "CURSES: Can't initialize screen. Returning\n");
        return -1;
    }

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
    wborder(stats, 0, 0, 0, 0, 0, 0, 0, 0);

    if (!stats || !map) {
        fprintf(stderr, "CURSES: Subwindow creation failed.");
        endwin();
        return -2;
    }

    game* gme = Initialize(MAP_WIDTH, MAP_HEIGHT);

    bool quit = false;
    while (!quit) {
        //  Read and process user input
        int input = getch();
        switch (input) {
            case 'w': ProcessInput(gme, INPUT_ROTATE); break;
            case 'a': ProcessInput(gme, INPUT_LEFT); break;
            case 's': ProcessInput(gme, INPUT_DOWN); break;
            case 'd': ProcessInput(gme, INPUT_RIGHT); break;
            case 'q': quit = true; break;
            default: break;
        }

        //  TODO: Refresh only when something has changed
        DrawMap(gme, map);
        DrawInfo(gme, stats);
        touchwin(win);  // Update game map
        // wrefresh(stats);
        // wrefresh(map);
        // refresh(); //   Update screen
    }
    FreeGame(gme);

    delwin(map);
    endwin();
    return 0;
}

void DrawMap(game* src, WINDOW* dst) {
    if (src == NULL) return;

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
        for (unsigned i = 0; i < 4; i++) {
            if (i == 0)
            mvwaddch(dst, src->active->y + mask[i]->y+1, src->active->x + mask[i]->x+1, '$');
            else
            mvwaddch(dst, src->active->y + mask[i]->y+1, src->active->x + mask[i]->x+1, 'O');
        }
    } else {
        mvaddch(0, 0, 'A');
    }
}

static void DrawInfo(game* src, WINDOW* dst) {
    game_info* s = &src->info;
    mvwprintw(dst, 1, 2, "Level: %d (%d)", s->level, s->rowsToNextLevel);
    mvwprintw(dst, 2, 2, "Score: %d", s->score);
}
