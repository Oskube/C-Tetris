#include <curses.h>

//  How many records are kept in the hiscore table
#define HISCORE_LENGTH 10
#define HISCORE_FILE "hiscores"

extern void* StateGame(WINDOW* win, void** data);
extern void* StateHiscores(WINDOW* win, void** data);
