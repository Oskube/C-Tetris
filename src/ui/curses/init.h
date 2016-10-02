#include "../ui.h"

/**
    \brief Initializes ncurses
    \param ret Pointer to struct used in returning function pointers
    \return 0 on success
*/
extern int CursesInit(UI_Functions* ret);

/**
    \brief Cleans and closes all ncurses related stuff
    \param ptr Pointer to struct used in returning function pointers
*/
extern void CursesCleanup(UI_Functions* ptr);
