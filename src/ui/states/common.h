#include "../ui.h"

/**
    \brief Displays game info

    Shows current score and other statistics for the given game instance.

    \param funs Pointer to UI functions struct
    \param gme Point to the game instance
    \param showNext Determine if next tetromino is shown
    \param out_topx Return left of info
    \param out_topy Return top of info
*/
extern void ShowGameInfo(UI_Functions* funs, game* gme, unsigned showNext, unsigned* out_topx, unsigned* out_topy);
