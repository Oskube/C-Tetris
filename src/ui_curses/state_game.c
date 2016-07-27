#include <stdio.h>
#include <sys/time.h> /* gettimeofday() */
#include <stdlib.h>

#include "states.h"
#include "../core/game.h"
#include "../core/hiscore.h"

#define MAP_WIDTH  10
#define MAP_HEIGHT 20

static void DrawMap(game* src, WINDOW* dst, bool showghost);
static void DrawInfo(game* src, WINDOW* dst);
static void DrawTetromino(tetromino* tetr, WINDOW* dst, unsigned topy, unsigned topx);
static void DrawStats(game* src, WINDOW* dst, unsigned topy, unsigned topx);
static unsigned GetTime(); /* Get milliseconds */

//  Static fsm functions
static int StateInit(WINDOW* win, void** data);
static void StateCleanUp();

//  Static vars used by this state
static bool is_running = false;
static WINDOW* map = NULL;
static WINDOW* stats = NULL;
static game* gme = NULL;

//  State code
void* StateGame(WINDOW* win, void** data) {

    //  State init
    if (!is_running) {
        if (StateInit(win, data) != 0) return NULL;
        is_running = true;
    }
    //  State code
    //  Read and process user input
    int input = getch();
    switch (input) {
        case 'w': ProcessInput(gme, INPUT_ROTATE); break;
        case 'a': ProcessInput(gme, INPUT_LEFT); break;
        case 's': ProcessInput(gme, INPUT_DOWN); break;
        case 'd': ProcessInput(gme, INPUT_RIGHT); break;
        case ' ': ProcessInput(gme, INPUT_SET); break;
        case 'q': is_running = false; break;
        default: break;
    }

    //  Update game logic
    Update(gme);

    //  TODO: Refresh only when something has changed
    //  Update UI
    DrawMap(gme, map, true);
    DrawInfo(gme, stats);

    //  If quit requested
    if (!is_running) {
        *data = malloc(sizeof(hiscore_list_entry));
        if (!*data) return NULL;

        //  Fill entry struct
        hiscore_list_entry* e = (hiscore_list_entry*)*data;
        e->score = gme->info.score;
        e->rows = gme->info.rows;
        e->lvl = gme->info.level;
        e->time = GetGameTime(gme);

        StateCleanUp();
        return StateHiscores;
    }

    return StateGame;   //  Continue with current state
}

int StateInit(WINDOW* win, void** data) {
    if (!data) return -1;

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
    map = subwin(win, MAP_HEIGHT+2, MAP_WIDTH+2, 1, 2);
    wborder(map, 0, 0, 0, 0, 0, 0, 0, 0);

    //  Sub window for stats and other info
    stats = subwin(win, winRows-2, winCols-MAP_WIDTH-8, 1, MAP_WIDTH+6);

    if (!stats || !map) {
        fprintf(stderr, "CURSES: Subwindow creation failed.");
        endwin();
        return -2;
    }

    gme = Initialize(MAP_WIDTH, MAP_HEIGHT, RANDOMISER_TGM, GetTime);
    if (!gme) {
        fprintf(stderr, "CORE: Couldn't initialize game");
        delwin(map);
        delwin(stats);
        endwin();
        return -3;
    }
    return 0;
}

/**
    \brief State cleanup
*/
void StateCleanUp() {
    FreeGame(gme);
    gme = NULL;
    delwin(stats);
    stats = NULL;
    delwin(map);
    map = NULL;
}

/*
    Other static function definitions
*/
void DrawMap(game* src, WINDOW* dst, bool showghost) {
    if (!src) return;

    //  First draw already set tetrominos
    unsigned w = src->map.width;
    unsigned h = src->map.height;
    block** mask = src->map.blockMask;
    const char symbols[7] = "0#$&8@%";

    unsigned len = w*h;
    unsigned row = 1;
    wmove(dst, row, 1);
    for (unsigned i = 0; i < len; i++) {
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
            mvwaddch(dst, y[0]+src->info.ghostY, x[0]+src->active->x, ':');
            mvwaddch(dst, y[1]+src->info.ghostY, x[1]+src->active->x, ':');
            mvwaddch(dst, y[2]+src->info.ghostY, x[2]+src->active->x, ':');
            mvwaddch(dst, y[3]+src->info.ghostY, x[3]+src->active->x, ':');
        }

        mvwaddch(dst, y[0]+src->active->y, x[0]+src->active->x, sym);
        mvwaddch(dst, y[1]+src->active->y, x[1]+src->active->x, sym);
        mvwaddch(dst, y[2]+src->active->y, x[2]+src->active->x, sym);
        mvwaddch(dst, y[3]+src->active->y, x[3]+src->active->x, sym);
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

unsigned GetTime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    unsigned ret = t.tv_sec*1000 + t.tv_usec/1000;
    return ret;
}
