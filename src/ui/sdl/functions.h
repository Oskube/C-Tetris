#include <stdbool.h>

#include "SDL.h"
#include "../ui.h"

typedef struct {
    SDL_Texture* texture;
    SDL_Rect* clips;
    unsigned len;
} SpriteSheet;

/**
    \brief Struct for all SDL related stuff needed in UI
*/
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    char* basePath;

    SDL_Rect* cell; /**< Size of a rendered character */
    unsigned gameAreaWidth; /**< Width of game area in pixels */

    //  DAS and ARR
    unsigned das; /**< Autorepeat start delay */
    unsigned arr; /**< Autorepeat rate */

    //  Game sprites
    SpriteSheet* borders;
    SpriteSheet* blocks;

    //  Font data
    SpriteSheet* font;
    char        font1st;
    char        fontlast;

    bool clearScreen;
    void* additional;
} ui_sdl_data;

extern int  UI_SDLGameInit(UI_Functions* funs);
extern void UI_SDLGameCleanUp(UI_Functions* funs);
extern int  UI_SDLGameRender(UI_Functions* funs, game* gme);
extern void UI_SDLBeginGameInfo(UI_Functions* funs, unsigned* x, unsigned* y);

extern void UI_SDLDemoShowPressed(UI_Functions* funs, unsigned topx, unsigned topy, demo_instruction* instruction);

extern void UI_SDLHiscoreRenderBegin(UI_Functions* funs);
extern int UI_SDLHiscoreGetName(UI_Functions* funs, hiscore_list_entry* entry, unsigned maxlen, unsigned rank);

extern void UI_SDLTextRender(UI_Functions* funs, unsigned x, unsigned y, text_color color, char* text);
extern void UI_SDLTetrominoRender(UI_Functions* funs, unsigned topx, unsigned topy, tetromino* tetr);
extern int  UI_SDLGetInput(UI_Functions* funs);
extern int  UI_SDLGetExePath(UI_Functions* funs, char* buf, unsigned len);
extern unsigned UI_SDLMillis();

extern void UI_SDLMainLoopEnd(UI_Functions* funs);

extern void AdjustCell(SDL_Rect* cell, unsigned winW, unsigned winH);
extern void FreeSpriteSheet(SpriteSheet* sheet);
