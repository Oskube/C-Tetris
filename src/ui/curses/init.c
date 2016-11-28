#include <stdlib.h> // malloc(), free()
#include <string.h> // strcmp()
#include <stdbool.h>
#include <curses.h>

#include "init.h"
#include "functions.h"
#include "../os/os.h"

static const char* helpStr =
"curses\n\
   --no-color\tDisables colors in terminal\n";

int CursesInit(UI_Functions* ret, int argc, char** argv) {
    bool ena_color = true;

    //  Process command line argments
    for (int pos = 1; pos < argc; pos++) {
        if (strcmp(argv[pos], "--no-color") == 0) {
            ena_color = false;
        }
    }

    //  Assign all function pointers
    ret->UIGameInit = GameWindowsInit;
    ret->UIGameCleanup = GameWindowsFree;
    ret->UIGameRender = GameRender;
    ret->UIBeginGameInfo = BeginGameInfo;

    ret->UIHiscoreRenderBegin = DrawHiscoresBegin;
    ret->UIHiscoreGetName = GetNewHiscoreName;

    ret->UITextRender = DrawText;
    ret->UITetrominoRender = DrawTetromino;
    ret->UIGetInput = ReadKey;
    ret->UIGetMillis = GetTime;
    ret->UIGetExePath = GetExePath;
    ret->UIMainLoopEnd = LoopEnd;

    ret->UICleanup = CursesCleanup;

    //  Initialize curses
    curses_data* newdata = (curses_data*)malloc(sizeof(curses_data));
    if (!newdata) return -1;
    newdata->win = initscr();
    if (!newdata->win) {
        return -2;
    }

    //  Initialize color support if available and wanted
    if (has_colors() && ena_color) {
        start_color();

        for (short i = 1; i < 8; i++)
            init_pair(i, i, 0);
    }

    ret->data = newdata;
    return 0;
}

void CursesCleanup(UI_Functions* ptr) {
    if(ptr && ptr->data) free(ptr->data);
    endwin();
}

const char* CursesGetHelp() {
    return helpStr;
}
