#include <stdio.h>
#include <string.h>

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "init.h"
#include "functions.h"

#define WIN_TITLE "CTetris"
#define WIN_DEF_W 800
#define WIN_DEF_H 600

/**
    \brief Load given image to SDL_Texture
    \param ren  SDL Rendering context
    \param path Path to image file
    \param outw Optional. Image width.
    \param outh Optional. Image height.
    \return NULL on error
*/
static SDL_Texture* LoadImage(SDL_Renderer* ren, const char* path, int* outw, int* outh);

/**
    \brief Clips given canvas into smaller rectangles
    \param canvas Rectangle to clip
    \param cell   Size of smaller rectangle
    \param outLen Length of array returned
    \return Array of SDL_Rect*. NULL on error

    \note Clips rectangle row-by-row
*/
static SDL_Rect* ClipRect(SDL_Rect* canvas, SDL_Rect* cell, unsigned *outLen);

int UI_SDLInit(UI_Functions* ret, int argc, char** argv) {
    //  Init SDL
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Could not initialize SDL:  %s\n", SDL_GetError());
        return -1;
    }

    //  Init window
    SDL_Window* win;
    win = SDL_CreateWindow(
        WIN_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WIN_DEF_W, WIN_DEF_H,
        SDL_WINDOW_RESIZABLE
    );
    if (!win) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return -2;
    }

    //  Init renderer
    SDL_Renderer* ren;
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return -2;
    }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    //  Set data pointer
    ui_sdl_data* sdldata = (ui_sdl_data*)malloc(sizeof(ui_sdl_data));
    if (!sdldata) {
        fprintf(stderr, "Could not create data container.\n");
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return -2;
    }

    /*  Load all used resources, fonts, images, audio etc.*/
    //  Load font
    //  Generate path
    char path[512] = {0};
    strcpy(path, SDL_GetBasePath());
    unsigned pathbaseLen = strlen(path);
    strncpy(path+pathbaseLen, "font.bmp", 512-pathbaseLen);

    //  Load image
    SDL_Rect canvas = {.x = 0, .y = 0 };
    SDL_Texture* font = LoadImage(ren, path, &canvas.w, &canvas.h);

    //  Clip
    unsigned clipLen = 0;
    SDL_Rect clip = {.w = 16, .h = 16 };
    SDL_Rect* fclips = ClipRect(&canvas, &clip, &clipLen);
    if (!fclips || !font) {
        fprintf(stderr, "Could not initialize font: %s\n", SDL_GetError());
        return -3;
    }

    //  Render cell rect
    sdldata->cell = (SDL_Rect*)calloc(1, sizeof(SDL_Rect));
    AdjustCell(sdldata->cell, WIN_DEF_W, WIN_DEF_H);

    //  Assign data
    ret->data = (void*)sdldata;
    sdldata->window = win;
    sdldata->renderer = ren;
    sdldata->basePath = SDL_GetBasePath();

    sdldata->font = font;
    sdldata->fontClips = fclips;
    sdldata->font1st = '!';
    sdldata->fontlast = '`';

    printf("Using base path: %s\n", sdldata->basePath);

    //  Assign all function pointers
    ret->UIGameInit = UI_SDLGameInit;
    ret->UIGameCleanup = UI_SDLGameCleanUp;
    ret->UIGameRender = UI_SDLGameRender;
    ret->UIBeginGameInfo = UI_SDLBeginGameInfo;

    ret->UIHiscoreRenderBegin = UI_SDLHiscoreRenderBegin;
    ret->UIHiscoreGetName = UI_SDLHiscoreGetName;

    ret->UITextRender = UI_SDLTextRender;
    ret->UITetrominoRender = UI_SDLTetrominoRender;
    ret->UIGetInput = UI_SDLGetInput;
    ret->UIGetMillis = UI_SDLMillis;
    ret->UIGetExePath = UI_SDLGetExePath;
    ret->UIMainLoopEnd = UI_SDLMainLoopEnd;

    ret->UICleanup = UI_SDLCleanUp;

    return 0;
}

void UI_SDLCleanUp(UI_Functions* ptr) {
    if (ptr && ptr->data) {
        ui_sdl_data* sdldata = (ui_sdl_data*)ptr->data;
        SDL_free(sdldata->basePath);

        //  Destroy font
        SDL_DestroyTexture(sdldata->font);
        free(sdldata->fontClips);
        free(sdldata->cell);

        //  Destroy rendere & window
        SDL_DestroyRenderer(sdldata->renderer);
        SDL_DestroyWindow(sdldata->window);
        free(sdldata);
    }
    SDL_Quit();
}

/**
    Static functions
*/

SDL_Texture* LoadImage(SDL_Renderer* ren, const char* path, int* outw, int* outh) {
    SDL_Texture* ret = NULL;
    SDL_Surface* surf = SDL_LoadBMP(path);
    if (surf) {
        SDL_SetColorKey(surf, SDL_TRUE, 0x00ff00ff); // set magenta transparent
        ret = SDL_CreateTextureFromSurface(ren, surf);
        if(ret) SDL_SetTextureBlendMode(ret, SDL_BLENDMODE_BLEND);

        if (outw) *outw = surf->w;
        if (outh) *outh = surf->h;
        SDL_FreeSurface(surf);
    }

    return ret;
}

SDL_Rect* ClipRect(SDL_Rect* canvas, SDL_Rect* cell, unsigned *outLen) {
    if (!canvas || !cell || !outLen) return NULL;
    //  Check for <= 0 in dimensions
    if (cell->w <= 0 || cell->h <= 0 || canvas->w <= 0 || canvas->h <= 0) return NULL;

    //  Count cells
    unsigned cellsX = canvas->w/cell->w;
    unsigned cellsY = canvas->h/cell->h;
    unsigned count = cellsX * cellsY;

    SDL_Rect* ret = (SDL_Rect*)malloc(sizeof(SDL_Rect)*count);
    if (!ret) return NULL;
    *outLen = count;

    unsigned x = canvas->x;
    unsigned y = canvas->y;
    for (unsigned cur = 0; cur < count; cur++) {
        int nextX = x + cell->w;
        if (nextX > canvas->w) {
            x = canvas->x;
            y += cell->y;
        }
        ret[cur].x = x;
        ret[cur].y = y;
        ret[cur].w = cell->w;
        ret[cur].h = cell->h;
        x = nextX;
    }

    return ret;
}
