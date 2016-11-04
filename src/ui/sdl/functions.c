#include "SDL.h"
#include "string.h" /* strlen() */
#include "functions.h"

#define WINDDOW_PADDING 20

typedef struct {
    unsigned char r, g, b;
} Color;

static Color sdl_colors[] = {
    {.r = 0, .g = 0, .b = 0 },      // 0, black
    {.r = 200, .g = 0, .b = 0 },    // 1, red
    {.r = 0, .g = 200, .b = 0 },    // 2, green
    {.r = 200, .g = 200, .b = 0 },  // 3, yellow
    {.r = 0, .g = 0, .b = 255 },    // 4, blue
    {.r = 255, .g = 0, .b = 255 },  // 5, magenta
    {.r = 0, .g = 255, .b = 255 },  // 6, cyan
    {.r = 255, .g = 255, .b = 255 },// 7, white
    {.r = 0, .g = 0, .b = 0},       // 8, default, should be something different than background
    //  Keep first 9 in this order, used in text rendering, same for all UIs

    {.r = 255, .g = 96, .b = 0 },   // 9, orange
};

//  Tetromino colors used with sdl_colors array
static int sym_colors[] = {
    // tetromino(color)
    // O(yellow), I(cyan), T(purple), L(orange), J(blue), S(green), Z(red)
    3, 6, 5, 9, 4, 2, 1
};

static void DrawTetromino(SDL_Renderer* ren, SDL_Rect* cel, tetromino* tetr, int offset_x, int offset_y);
static void DrawMap(SDL_Renderer* ren, SDL_Rect* dstArea, game* gme);

int UI_SDLGameInit(UI_Functions* funs) {

    return 0;
}

void UI_SDLGameCleanUp(UI_Functions* funs) {

}

int UI_SDLGameRender(UI_Functions* funs, game* gme) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    data->clearScreen = true; // request clean screen after rendering
    int winW, winH;
    SDL_GetWindowSize(data->window, &winW, &winH);
    SDL_Rect gameArea = {
        .x = WINDDOW_PADDING, .y = WINDDOW_PADDING,
        .w = winW*0.2f, .h = winH - WINDDOW_PADDING*2
    };
    if (gameArea.h <= 0) gameArea.h = 1; // Height cannot be <0
    DrawMap(data->renderer, &gameArea,gme);
    return 0;
}

void UI_SDLHiscoreRenderBegin(UI_Functions* funs) {
    //  Make sure screen is clear
    ui_sdl_data* data = (ui_sdl_data*)funs->data;
}

void UI_SDLHiscoreGetName(UI_Functions* funs, hiscore_list_entry* entry, unsigned maxlen, unsigned rank) {

}

void UI_SDLTextRender(UI_Functions* funs, unsigned x, unsigned y, text_color color, char* text) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    int winW, winH;
    SDL_GetWindowSize(data->window, &winW, &winH);
    int rowSz = winW/24;
    int colSz = winW/80;

    unsigned len = strlen(text);

    //  Set text color
    SDL_SetTextureColorMod(data->font, sdl_colors[color].r, sdl_colors[color].g, sdl_colors[color].b);
    SDL_SetTextureAlphaMod(data->font, 255);

    //  Text rendering
    SDL_Rect target = {.x = x*colSz, .y = y*rowSz, .w = colSz, .h = colSz};
    for (unsigned i = 0; i < len; i++, target.x += colSz) {
        int ch = toupper(text[i]);

        if (ch <= data->fontlast) {
            int pos = ch - data->font1st;
            if (pos >= 0) {
                SDL_RenderCopy(data->renderer, data->font, &data->fontClips[pos], &target);
            }
        }
    }
}

int UI_SDLGetInput(UI_Functions* funs) {
    SDL_Event ev;
    int ret = 0;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT: return 'q';
            case SDL_KEYDOWN: {
                switch (ev.key.keysym.scancode) {
                    case SDL_SCANCODE_SPACE: return ' ';
                    default: {
                        char* key = SDL_GetKeyName(SDL_GetKeyFromScancode(ev.key.keysym.scancode));
                        //   All letters and numbers (excluding numpad-nums)
                        if (strlen(key) == 1) {
                            // printf("Button down: %c\n", key[0]);
                            return key[0];
                        }
                    } break;
                }
            } break;
            default: break;
        }
    }
    return 0;
}

int UI_SDLGetExePath(UI_Functions* funs, char* buf, unsigned len) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;
    char* bPath = data->basePath;

    if (strlen(bPath) < len) {
        strcpy(buf, bPath);
        return strlen(bPath);
    } else {
        return -1;
    }
}

unsigned UI_SDLMillis() {
    return SDL_GetTicks();
}

void UI_SDLMainLoopEnd(UI_Functions* funs) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    SDL_RenderPresent(data->renderer);
    SDL_SetRenderDrawColor(data->renderer, 255, 255, 255, 255);
    if (data->clearScreen) {
        SDL_RenderClear(data->renderer);
        data->clearScreen = false;
    }
}

/***********************
Static functions
***********************/
void DrawMap(SDL_Renderer* ren, SDL_Rect* dstArea, game* gme) {
    SDL_Rect cell = {
        .w = dstArea->w/MAP_WIDTH, .h = dstArea->h/MAP_HEIGHT,
        .x = dstArea->x, .y = dstArea->y
    };

    //  Make sure destination area is even size
    dstArea->w = cell.w*MAP_WIDTH;
    dstArea->h = cell.h*MAP_HEIGHT;
    //  Draw borders
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderDrawRect(ren, dstArea);

    unsigned w = gme->map.width;
    unsigned h = gme->map.height;
    block** mask = gme->map.blockMask;
    unsigned len = w*h;
    int rightBorder = dstArea->x+dstArea->w;

    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
    for (unsigned pos = w*2; pos < len; pos++) {
        //  If block exists
        if (mask[pos]) {
            //  Change color and render
            unsigned sym = mask[pos]->symbol;
            sym = sym_colors[sym];
            SDL_SetRenderDrawColor(ren, sdl_colors[sym].r, sdl_colors[sym].g, sdl_colors[sym].b, 255);
            SDL_RenderFillRect(ren, &cell);
        }

        cell.x += cell.w;
        if (cell.x >= rightBorder) { // If row processed move to next
            cell.x = dstArea->x;
            cell.y += cell.h;
        }
    }


    unsigned c = gme->active->blocks[0]->symbol;
    c = sym_colors[c];
    SDL_SetRenderDrawColor(ren, sdl_colors[c].r, sdl_colors[c].g, sdl_colors[c].b, 255);
    DrawTetromino(ren, &cell, gme->active, dstArea->x, dstArea->y);
    unsigned tmp = gme->active->y;

    //  Ghost
    gme->active->y = gme->info.ghostY;
    SDL_SetRenderDrawColor(ren, sdl_colors[c].r, sdl_colors[c].g, sdl_colors[c].b, 64);
    DrawTetromino(ren, &cell, gme->active, dstArea->x, dstArea->y);
    gme->active->y = tmp;
}

void DrawTetromino(SDL_Renderer* ren, SDL_Rect* cell, tetromino* tetr, int offset_x, int offset_y) {
    if (!tetr || !ren) return;

    unsigned origoX = tetr->x;
    unsigned origoY = tetr->y -2; // 2 hidden rows

    SDL_Rect temp = *cell;
    block** blocks = tetr->blocks;
    for (unsigned i = 0; i < 4; i++) {
        temp.x = cell->w * (blocks[i]->x +origoX) +offset_x;
        temp.y = cell->h * (blocks[i]->y +origoY) +offset_y;

        //  Determine if block is in game area
        if (temp.y >= offset_y) SDL_RenderFillRect(ren, &temp);
    }
}
