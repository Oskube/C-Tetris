#ifndef _UI_H_
#define _UI_H_

//  Find better place for these
#define MAP_WIDTH  10
#define MAP_HEIGHT 20

#include "../core/game.h"
#include "../core/hiscore.h"

extern int MainProgram(int argc, char** argv);

/**
    \brief Struct containing pointers to functions used by UI
*/
typedef struct _uifun {
    //  Game
    int  (*UIGameInit)(struct _uifun*);     /**< Initializes game state UI */
    void (*UIGameCleanup)(struct _uifun*);
    int  (*UIGameRender)(struct _uifun*, game*);   /**< Renders current game view */

    //  High scores
    void (*UIHiscoreRenderBegin)(struct _uifun*); /**< Beginning of render hiscore view loop */
    void (*UIHiscoreGetName)(struct _uifun*, hiscore_list_entry* entry, unsigned maxlen, unsigned rank); /**< Asks user name */

    //  Common
    void (*UITextRender)(struct _uifun*, unsigned x, unsigned y, unsigned color, char* text); /**< Renders text to UI */
    int  (*UIGetInput)(struct _uifun*);         /**< Read user input */
    unsigned (*UIGetMillis)();                      /**< Return milliseconds since start up */
    int (*UIGetExePath)(struct _uifun*, char* buf, unsigned len);   /**< Return path to executable */
    void (*UIMainLoopEnd)(struct _uifun*);          /**< Function called at the end of the main loop */

    //  Internal
    void (*UICleanup)(struct _uifun*); /**< Frees memory and closes ui */

    void* data; /**< Pointer to data used by rendering functions */
} UI_Functions;

#endif //_UI_H_
