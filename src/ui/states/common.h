#include "../ui.h"

/**
    \brief Displays game info

    Shows current score and other statistics for the given game instance.

    \param funs Pointer to UI functions struct
    \param gme Point to the game instance
    \param showNext Determine if next tetromino is shown
*/
extern void ShowGameInfo(UI_Functions* funs, game* gme, unsigned showNext);
