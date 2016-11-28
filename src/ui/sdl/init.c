#include <stdio.h> // malloc(), atoi()
#include <stdbool.h>
#include <string.h> // stcmp(), strlen(), strcpy(), strncpy()

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
static SDL_Rect* ClipRectBySize(SDL_Rect* canvas, SDL_Rect* cell, unsigned *outLen);

/**
    \brief Clip given canvas to smaller rectangles
    \param canvas   Area to clip
    \param x        Count of sprite columns
    \param y        Count of sprite rows
    \return Array of SDL_Rect*. NULL on error

    \note Returns x*y of even sized rectangles
*/
static SDL_Rect* ClipRectByCount(SDL_Rect* canvas, unsigned x, unsigned y);
/**
    \brief Returns SpriteSheet struct with given data
    \param texture Pointer to the texture
    \param clips   Pointer to clips
    \param len     Sprite count
    \return Pointer to the allocated SpriteSheet

    \note Remember to free allocated memory!
    \see FreeSpriteSheet(SpriteSheet* sheet)
*/
static SpriteSheet* GenerateSheet(SDL_Texture* texture, SDL_Rect* clips, unsigned len);

static const char* CmdLineHelpStr =
" SDL\n\
   --no-textures\t\tDisables texture loading, except for font\n\
   --width <w>, --height <h>\tSet window size\n";

int UI_SDLInit(UI_Functions* ret, int argc, char** argv) {
    bool ena_textures = true;
    int winWidth = WIN_DEF_W, winHeight = WIN_DEF_H;

    //  Process command line arguments
    for (int pos = 1; pos < argc; pos++) {
        if (strcmp(argv[pos], "--no-textures") == 0) {
            ena_textures = false;
        }
        else if (strcmp(argv[pos], "--width") == 0) {
            if (++pos >= argc) continue; // ignore if no value
            winWidth = atoi(argv[pos]);
            if (winWidth < 10) winWidth = 10;
        }
        else if (strcmp(argv[pos], "--height") == 0) {
            if (++pos >= argc) continue; // ignore if no value
            winHeight = atoi(argv[pos]);
            if (winHeight < 10) winHeight = 10;
        }
    }

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
        winWidth, winHeight,
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
    SDL_Rect clip = {.w = 16, .h = 16 };
    SDL_Rect* fclips = ClipRectBySize(&canvas, &clip, NULL);
    if (!fclips || !font) {
        fprintf(stderr, "Could not initialize font. %s\n", SDL_GetError());
        return -3;
    }

    //  Generate font sprite sheet
    sdldata->font = GenerateSheet(font, fclips, 0);

    sdldata->font1st = '!';
    sdldata->fontlast = '`';

    sdldata->borders = NULL;
    sdldata->blocks = NULL;
    if (ena_textures) {
        //  Load game area background and borders
        strncpy(path+pathbaseLen, "borders-hires.bmp", 512-pathbaseLen);
        SDL_Texture* borders = LoadImage(ren, path, &canvas.w, &canvas.h);
        SDL_Rect* border_clips = ClipRectByCount(&canvas, 3, 3);
        if (!borders || !border_clips) {
            fprintf(stderr, "Could not load borders. %s\n", SDL_GetError());
            return -4;
        }
        sdldata->borders = GenerateSheet(borders, border_clips, 0);

        //  Load block textures
        strncpy(path+pathbaseLen, "blocks.bmp", 512-pathbaseLen);
        SDL_Texture* blocks = LoadImage(ren, path, &canvas.w, &canvas.h);
        SDL_Rect* block_clips = ClipRectByCount(&canvas, 7, 1);
        if (!blocks || !block_clips) {
            fprintf(stderr, "Could not load block texture. %s\n", SDL_GetError());
            return -4;
        }

        sdldata->blocks = GenerateSheet(blocks, block_clips, 0);
    } // ena_textures

    //  Render cell rect
    sdldata->cell = (SDL_Rect*)calloc(1, sizeof(SDL_Rect));
    AdjustCell(sdldata->cell, winWidth, winHeight);

    //  Assign data
    ret->data = (void*)sdldata;
    sdldata->window = win;
    sdldata->renderer = ren;
    sdldata->basePath = SDL_GetBasePath();

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

        FreeSpriteSheet(sdldata->blocks);
        FreeSpriteSheet(sdldata->borders);
        //  Destroy font
        FreeSpriteSheet(sdldata->font);
        free(sdldata->cell);

        //  Destroy rendere & window
        SDL_DestroyRenderer(sdldata->renderer);
        SDL_DestroyWindow(sdldata->window);
        free(sdldata);
    }
    SDL_Quit();
}

const char* UI_SDLGetHelp() {
    return CmdLineHelpStr;
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

SDL_Rect* ClipRectBySize(SDL_Rect* canvas, SDL_Rect* cell, unsigned *outLen) {
    if (!canvas || !cell) return NULL;
    //  Check for <= 0 in dimensions
    if (cell->w <= 0 || cell->h <= 0 || canvas->w <= 0 || canvas->h <= 0) return NULL;

    //  Count cells
    unsigned cellsX = canvas->w/cell->w;
    unsigned cellsY = canvas->h/cell->h;
    unsigned count = cellsX * cellsY;

    SDL_Rect* ret = (SDL_Rect*)malloc(sizeof(SDL_Rect)*count);
    if (!ret) return NULL;
    if (outLen) *outLen = count;

    unsigned x = 0;
    unsigned y = 0;
    for (unsigned cur = 0; cur < count; cur++, x = x+cell->w) {
        int nextX = x + cell->w;
        if (nextX > canvas->w) {
            x = 0;
            y += cell->h;
        }
        ret[cur].x = x + canvas->x;
        ret[cur].y = y + canvas->y;
        ret[cur].w = cell->w;
        ret[cur].h = cell->h;
    }

    return ret;
}

SDL_Rect* ClipRectByCount(SDL_Rect* canvas, unsigned x, unsigned y) {
    if (!canvas) return NULL;

    //  Calculate step
    int sX = canvas->w, sY = canvas->h;
    if (x>0) sX /= x;
    if (y>0) sY /= y;

    SDL_Rect* ret = (SDL_Rect*)malloc(sizeof(SDL_Rect)*x*y);
    if (!ret) return NULL;

    //  Clipping
    int posx = 0, posy = 0;
    for (unsigned cur = 0; cur < x*y; posx += sX, cur++) {
        if (posx >= canvas->w) {
            posx = 0;
            posy += sY;
        }
        ret[cur].x = posx + canvas->x;
        ret[cur].y = posy + canvas->y;
        ret[cur].w = sX;
        ret[cur].h = sY;
    }

    return ret;
}


static SpriteSheet* GenerateSheet(SDL_Texture* texture, SDL_Rect* clips, unsigned len) {
    SpriteSheet* sheet = (SpriteSheet*)malloc(sizeof(SpriteSheet));
    if (sheet) {
        sheet->texture = texture;
        sheet->clips = clips;
        sheet->len = len;
    }

    return sheet;
}
