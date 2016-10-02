#include <stdlib.h>
#include <curses.h>

#include "curses_core.h"
#include "curses_functions.h"
#include "os_dependent.h"

int CursesInit(UI_Functions* ret) {
    //  Assign all function pointers
    ret->UIGameInit = GameWindowsInit;
    ret->UIGameCleanup = GameWindowsFree;
    ret->UIGameRender = GameRender;

    ret->UIHiscoreRender = DrawHiscores;
    ret->UIHiscoreGetName = GetNewHiscoreName;

    ret->UITextRender = DrawText;
    ret->UIGetInput = ReadKey;
    ret->UIGetMillis = GetTime;
    ret->UIGetExePath = GetExecutablePath;
    ret->UIMainLoopEnd = LoopEnd;

    ret->UICleanup = CursesCleanup;

    //  Initialize curses
    curses_data* newdata = (curses_data*)malloc(sizeof(curses_data));
    if (!newdata) return -1;
    newdata->win = initscr();
    if (!newdata->win) {
        return -2;
    }
    ret->data = newdata;
    return 0;
}

void CursesCleanup(UI_Functions* ptr) {
    if(ptr && ptr->data) free(ptr->data);
    endwin();
}
