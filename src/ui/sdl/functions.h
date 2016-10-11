#include "SDL.h"
#include "../ui.h"

/**
    \brief Struct for all SDL related stuff needed in UI
*/
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    char* basePath;
    //  Font
    SDL_Texture* font;
    SDL_Rect*    fontClips;
    char         font1st;
    char         fontlast;

    void* additional;
} ui_sdl_data;

extern int  UI_SDLGameInit(UI_Functions* funs);
extern void UI_SDLGameCleanUp(UI_Functions* funs);
extern int  UI_SDLGameRender(UI_Functions* funs, game* gme);

extern void UI_SDLHiscoreRender(UI_Functions* funs, hiscore_list_entry* list, unsigned len);
extern void UI_SDLHiscoreGetName(UI_Functions* funs, hiscore_list_entry* entry, unsigned maxlen, unsigned rank);

extern void UI_SDLTextRender(UI_Functions* funs, unsigned x, unsigned y, unsigned color, char* text);
extern int  UI_SDLGetInput(UI_Functions* funs);
extern int  UI_SDLGetExePath(UI_Functions* funs, char* buf, unsigned len);
extern unsigned UI_SDLMillis();

extern void UI_SDLMainLoopEnd(UI_Functions* funs);
