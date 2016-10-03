#include <stdio.h>

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "init.h"
#include "functions.h"

#define WIN_TITLE "CTetris"
#define WIN_DEF_W 800
#define WIN_DEF_H 600


int UI_SDLInit(UI_Functions* ret) {
    //  Init SDL
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "\nCould not initialize SDL:  %s\n", SDL_GetError());
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
        fprintf(stderr, "\nCould not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return -2;
    }

    //  Init renderer
    SDL_Renderer* ren;
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        fprintf(stderr, "\nCould not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return -2;
    }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    /*  Load all used resources, fonts, images, audio etc.*/

    //  Set data pointer
    ui_sdl_data* sdldata = (ui_sdl_data*)malloc(sizeof(ui_sdl_data));
    if (!sdldata) {
        fprintf(stderr, "\nCould not create renderer: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return -2;
    }
    ret->data = (void*)sdldata;
    sdldata->window = win;
    sdldata->renderer = ren;
    sdldata->basePath = SDL_GetBasePath();
    printf("Using base path: %s\n", sdldata->basePath);

    //  Assign all function pointers
    ret->UIGameInit = UI_SDLGameInit;
    ret->UIGameCleanup = UI_SDLGameCleanUp;
    ret->UIGameRender = UI_SDLGameRender;

    ret->UIHiscoreRender = UI_SDLHiscoreRender;
    ret->UIHiscoreGetName = UI_SDLHiscoreGetName;

    ret->UITextRender = UI_SDLTextRender;
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
        SDL_DestroyRenderer(sdldata->renderer);
        SDL_DestroyWindow(sdldata->window);
    }
    SDL_Quit();
}
