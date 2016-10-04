#include "SDL.h"
#include "string.h" /* strlen() */
#include "functions.h"

#define WINDDOW_PADDING 20

typedef struct {
    unsigned char r, g, b;
} SymbolColor;

static SymbolColor sym_color[] = {
    {.r = 255, .g = 255, .b = 0 },  // SHAPE_O = yellow
    {.r = 0, .g = 255, .b = 255 },  // SHAPE_I = cyan
    {.r = 255, .g = 0, .b = 255 },  // SHAPE_T = purple
    {.r = 255, .g = 96, .b = 0 },   // SHAPE_L = orange
    {.r = 0, .g = 0, .b = 255 },    // SHAPE_J = blue
    {.r = 0, .g = 255, .b = 0 },    // SHAPE_S = green
    {.r = 255, .g = 0, .b = 0 },    // SHAPE_Z = red
    {.r = 0, .g = 0, .b = 0 },      // SHAPE_MAX
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

void UI_SDLHiscoreRender(UI_Functions* funs, hiscore_list_entry* list, unsigned len) {

}

void UI_SDLHiscoreGetName(UI_Functions* funs, hiscore_list_entry* entry, unsigned maxlen, unsigned rank) {

}

void UI_SDLTextRender(UI_Functions* funs, unsigned x, unsigned y, char* text) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    int winW, winH;
    SDL_GetWindowSize(data->window, &winW, &winH);
    int rowSz = winW/24;
    int colSz = winW/80;

    unsigned len = strlen(text);
    SDL_Rect target = {.x = x*colSz, .y = y*rowSz, .w = colSz, .h = rowSz};
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
    SDL_RenderClear(data->renderer);
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
            SDL_SetRenderDrawColor(ren, sym_color[sym].r, sym_color[sym].g, sym_color[sym].b, 255);
            SDL_RenderFillRect(ren, &cell);
        }

        cell.x += cell.w;
        if (cell.x >= rightBorder) { // If row processed move to next
            cell.x = dstArea->x;
            cell.y += cell.h;
        }
    }


    unsigned sym = gme->active->blocks[0]->symbol;
    SDL_SetRenderDrawColor(ren, sym_color[sym].r, sym_color[sym].g, sym_color[sym].b, 255);
    DrawTetromino(ren, &cell, gme->active, dstArea->x, dstArea->y);
    unsigned tmp = gme->active->y;

    //  Ghost
    gme->active->y = gme->info.ghostY;
    SDL_SetRenderDrawColor(ren, sym_color[sym].r, sym_color[sym].g, sym_color[sym].b, 64);
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
