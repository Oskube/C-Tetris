#include "SDL.h"
#include "string.h" /* strlen() */
#include "functions.h"

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
    {.r = 255, .g = 255, .b = 255}, // 8, default, should be something different than background
    //  Keep first 9 in this order, used in text rendering, same for all UIs

    {.r = 255, .g = 96, .b = 0 },   // 9, orange
};

//  Tetromino colors used with sdl_colors array
static int sym_colors[] = {
    // tetromino(color)
    // O(yellow), I(cyan), T(purple), L(orange), J(blue), S(green), Z(red)
    3, 6, 5, 9, 4, 2, 1
};

static unsigned movementKeys = 0; /** Bitmask to save movement key states for eliminating repeat delay */

//  Static function declarations -------------
static void DrawTetromino(SDL_Renderer* ren, SDL_Rect* cel, SpriteSheet* texture, tetromino* tetr, int offset_x, int offset_y, bool ignorearea);
static int HandleEvents(UI_Functions* funs, SDL_Event* ev);

/**
    \brief Render game area
    \param sdldata Pointer to SDL UI data
    \param gme     Pointer to the game instance
*/
static void DrawMap(UI_Functions* funs, game* gme);

/**
    \brief Render a box with given sprite sheet

    Draws a box of given size. Size of box are determined by target, not including borders.
    \param ren      Pointer to SDL renderer
    \param cell     Dimensions of the sprite element
    \param style    Pointer to the sprite sheet
    \param target   Dimensions of the box contents in pixels
*/
static void RenderBox(SDL_Renderer* ren, SDL_Rect* cell, SpriteSheet* style, SDL_Rect* target);

/**
    \brief Render sprite from a sheet
    \param ren          SDL Renderer used
    \param sheet        Pointer to sprite sheet
    \param clip         Index of the sprite
    \param pos_target   Area where sprite is rendered
*/
static void RenderSprite(SDL_Renderer* ren, SpriteSheet* sheet, unsigned clip, SDL_Rect* pos_target);

int UI_SDLGameInit(UI_Functions* funs) {
    // ui_sdl_data* data = (ui_sdl_data*)funs->data;

    return 0;
}

void UI_SDLGameCleanUp(UI_Functions* funs) {
    //  Free everything allocated during the game state by UI component
}

int UI_SDLGameRender(UI_Functions* funs, game* gme) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    data->clearScreen = true; // request clean screen after rendering

    DrawMap(funs, gme);
    return 0;
}

void UI_SDLBeginGameInfo(UI_Functions* funs, unsigned* x, unsigned* y) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    if (data->gameAreaWidth == 0) *x = 1;
    else *x = data->gameAreaWidth/data->cell->w + 2;
    *y = 1;
}

void UI_SDLDemoShowPressed(UI_Functions* funs, unsigned topx, unsigned topy, demo_instruction* instruction) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    unsigned ticks = SDL_GetTicks();
    static unsigned pressed[INPUT_SET+1] = {0}; // left, right, down, rotate, set
    if (instruction) {
        unsigned instr = instruction->instruction;
        if (instr <= INPUT_SET) {
            pressed[instr] = ticks;
        }
    }

    //  Draw keys
    unsigned cell = data->cell->h * 2;
    SDL_Rect target = {.x = topx*data->cell->w, .y = topy*data->cell->h, .w = cell, .h = cell};
    for(unsigned i=0; i <= INPUT_SET; i++) {
        switch (i) {
            case INPUT_LEFT: {
                target.y += cell;
            } break;
            case INPUT_RIGHT: {
                target.x += cell*2;
            } break;
            case INPUT_DOWN: {
                target.x -= cell;
            } break;
            case INPUT_ROTATE: {
                target.y -= cell;
            } break;
            case INPUT_SET: {
                target.x += cell*3;
                target.y += cell;
            } break;
            default: break;
        }

        int delta = pressed[i] + 128 - ticks;
        SDL_SetRenderDrawColor(data->renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(data->renderer, &target);

        if (delta > 0) {
            SDL_SetRenderDrawColor(data->renderer, 255, 255, 255, delta << 1);
            SDL_RenderFillRect(data->renderer, &target);
        }
    }
}

void UI_SDLHiscoreRenderBegin(UI_Functions* funs) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    //  Make sure screen is clear
    SDL_SetRenderDrawColor(data->renderer, 64, 64, 64, 255);
    SDL_RenderClear(data->renderer);
}

int UI_SDLHiscoreGetName(UI_Functions* funs, hiscore_list_entry* entry, unsigned maxlen, unsigned rank) {
    //  Generate message
    char msg[128] = {0};
    snprintf(msg, 128, "NEW HISCORE, %d. PLACE.", rank);

    SDL_StartTextInput();
    SDL_Event ev;

    unsigned drawCaret = (SDL_GetTicks() & 0x3FF) >> 9;

    int ret = 0;
    if (SDL_PollEvent(&ev)) {
        //  Return
        if (ev.type == SDL_KEYUP && ev.key.keysym.scancode == SDL_SCANCODE_RETURN) {
            funs->inputs[0] = event_ready;
            ret = 1;
        }
        //  Erasing
        else if (ev.type == SDL_KEYDOWN && ev.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
            unsigned len = strlen(entry->name);
            if (len > 0) {
                entry->name[len-1] = '\0';
            }
        }
        else if (ev.type == SDL_TEXTINPUT) {
            //  Append text input
            if (strlen(entry->name) < maxlen) {
                strncat(entry->name, ev.text.text, 1);
            }
        } else {
            //  Handle other events
            ret = HandleEvents(funs, &ev);
            if (!isalpha(ret)) {
                funs->inputs[0] = ret;
                ret = 1;
            } else {
                ret = 0;
            }
        }
    }

    //  Render text
    UI_SDLTextRender(funs, 30, 15, color_red, msg);
    UI_SDLTextRender(funs, 35, 16, color_red, entry->name);

    if (drawCaret) {
        ui_sdl_data* data = (ui_sdl_data*)funs->data;
        SDL_Rect caret = *(data->cell);
        caret.x = (strlen(entry->name)+35) * caret.w;
        caret.y = caret.h * 16;

        SDL_SetRenderDrawColor(data->renderer, sdl_colors[1].r, sdl_colors[1].g, sdl_colors[1].b, 255);
        SDL_RenderFillRect(data->renderer, &caret);
        SDL_SetRenderDrawColor(data->renderer, 255, 255, 255, 255);
    }

    return ret;
}

void UI_SDLTextRender(UI_Functions* funs, unsigned x, unsigned y, text_color color, char* text) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    int winW, winH;
    SDL_GetWindowSize(data->window, &winW, &winH);
    int rowSz = data->cell->h;
    int colSz = data->cell->w;

    unsigned len = strlen(text);

    //  Set text color
    SDL_SetTextureColorMod(data->font->texture, sdl_colors[color].r, sdl_colors[color].g, sdl_colors[color].b);
    SDL_SetTextureAlphaMod(data->font->texture, 255);

    //  Text rendering
    SDL_Rect target = {.x = x*colSz, .y = y*rowSz, .w = colSz, .h = rowSz};
    for (unsigned i = 0; i < len; i++, target.x += colSz) {
        int ch = toupper(text[i]);

        if (ch <= data->fontlast) {
            int pos = ch - data->font1st;
            if (pos >= 0) {
                RenderSprite(data->renderer, data->font, pos, &target);
            }
        }
    }
}

void UI_SDLTetrominoRender(UI_Functions* funs, unsigned topx, unsigned topy, tetromino* tetr) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    unsigned c = tetr->blocks[0]->symbol;
    c = sym_colors[c];
    SDL_SetRenderDrawColor(data->renderer, sdl_colors[c].r, sdl_colors[c].g, sdl_colors[c].b, 255);

    SDL_Rect cell = *data->cell;
    cell.w = cell.h;
    DrawTetromino(data->renderer, &cell, data->blocks, tetr, data->cell->w*topx, (data->cell->h)*(topy), true);
}

int UI_SDLGetInput(UI_Functions* funs) {
    if (!funs) return 0;
    unsigned old_movementKeys = movementKeys;

    SDL_Event ev;
    unsigned count = 0;
    //  Process events if any and array not full
    while (SDL_PollEvent(&ev) && count < INPUT_ARRAY_LEN) {
        int input = HandleEvents(funs, &ev);
        if (input == 0) continue; // nothing interesting

        funs->inputs[count] = input;
        count++;
    }

    static unsigned next;
    //  If movement keys are down add them, "wasd"-keys
    if (movementKeys) {
        bool getInput = false;

        ui_sdl_data* data = funs->data;
        if (old_movementKeys < movementKeys) {
            // More keys down than last time, next repeat is t+DAS
            next = SDL_GetTicks() + data->das;
            getInput = true;
        } else if (next < SDL_GetTicks()) {
            // Next repeat is t+ARR
            next = SDL_GetTicks() + data->arr;
            getInput = true;
        }

        if (getInput) {
            for(unsigned pos = 0; pos < movementKeys && count < INPUT_ARRAY_LEN; pos++) {
                char keytocmd[] = "adsw";
                if (movementKeys & (1 << pos)) {
                    funs->inputs[count] = keytocmd[pos];
                    count++;
                }
            }
        }
    }

    return count;
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
    SDL_SetRenderDrawColor(data->renderer, 64, 64, 64, 255);
    if (data->clearScreen) {
        SDL_RenderClear(data->renderer);
        data->clearScreen = false;
    }

    SDL_Delay(2); // Sleep 2 ms to reduce CPU usage
}

void AdjustCell(SDL_Rect* cell, unsigned winW, unsigned winH) {
    // 80x24 cells in a window
    cell->x = 0;
    cell->y = 0;
    cell->w = winW / 80;
    cell->h = winH / 24;

    //  Make sure cell can't be 0
    if (cell->w == 0) cell->w = 1;
    if (cell->h == 0) cell->h = 1;
}

void FreeSpriteSheet(SpriteSheet* sheet) {
    if (!sheet) return; // nothing to free

    free(sheet->clips);
    SDL_DestroyTexture(sheet->texture);

    sheet->texture = NULL;
    sheet->clips = NULL;
    free(sheet);
}

/***********************
Static functions
***********************/
void DrawMap(UI_Functions* funs, game* gme) {
    ui_sdl_data* sdldata = (ui_sdl_data*)funs->data;
    SDL_Renderer* ren = sdldata->renderer;
    SDL_Rect cell = *sdldata->cell;
    cell.w = cell.h;    //  Game area cells are squares

    //  Calculate game area
    SDL_Rect target = cell;
    target.x = target.w;
    target.y = target.h;
    target.w *= MAP_WIDTH;
    target.h *= MAP_HEIGHT;
    RenderBox(ren, &cell, sdldata->borders, &target); // Draw bg and borders
    sdldata->gameAreaWidth = target.w+target.x; //  Set gameAreaWidth in pixels

    //  Don't render blocks if paused
    if (gme->info.status & GAME_STATUS_PAUSE) {
        SDL_Rect cell = *sdldata->cell;
        //  Center the text
        unsigned x = (target.x + target.w) / cell.w / 2 - 4;
        unsigned y = (target.y + target.h) / cell.h / 2;
        UI_SDLTextRender(funs, x, y, color_white, "Game paused!");
        return;
    }

    //  Game area blocks
    block** mask = gme->map.blockMask;
    unsigned len =  gme->map.width*gme->map.height;
    int rightBorder = target.x+target.w;
    cell.x = target.x;
    cell.y = target.y;

    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
    //  Render the whole game area, except the 2 top rows which are hidden
    for (unsigned pos = gme->map.width*2; pos < len; pos++) {
        //  If block exists
        if (mask[pos]) {
            //  Change color and render
            unsigned sym = mask[pos]->symbol;

            // if texture loaded, use it
            if (sdldata->blocks) {
                RenderSprite(ren, sdldata->blocks, sym, &cell);
            } else {
                sym = sym_colors[sym];
                SDL_SetRenderDrawColor(ren, sdl_colors[sym].r, sdl_colors[sym].g, sdl_colors[sym].b, 255);
                SDL_RenderFillRect(ren, &cell);
            }
        }

        cell.x += cell.w;
        if (cell.x >= rightBorder) { // If row processed move to the next row
            cell.x = target.x;
            cell.y += cell.h;
        }
    }

    //  Render active tetromino
    unsigned c = gme->active->blocks[0]->symbol;
    c = sym_colors[c];
    SDL_SetRenderDrawColor(ren, sdl_colors[c].r, sdl_colors[c].g, sdl_colors[c].b, 255);
    DrawTetromino(ren, &cell, sdldata->blocks, gme->active, target.x, target.y, false);
    unsigned tmp = gme->active->y;

    //  Render ghost
    gme->active->y = gme->info.ghostY;
    if (sdldata->blocks) {
        SDL_SetTextureAlphaMod(sdldata->blocks->texture, 64);
        DrawTetromino(ren, &cell, sdldata->blocks, gme->active, target.x, target.y, false);
        SDL_SetTextureAlphaMod(sdldata->blocks->texture, 255);
    } else {
        SDL_SetRenderDrawColor(ren, sdl_colors[c].r, sdl_colors[c].g, sdl_colors[c].b, 64);
        DrawTetromino(ren, &cell, sdldata->blocks, gme->active, target.x, target.y, false);
    }
    gme->active->y = tmp;
}

void DrawTetromino(SDL_Renderer* ren, SDL_Rect* cell, SpriteSheet* texture, tetromino* tetr, int offset_x, int offset_y, bool ignorearea) {
    if (!tetr || !ren || !cell) return;

    unsigned origoX = tetr->x;
    unsigned origoY = tetr->y -2; // 2 hidden rows

    SDL_Rect temp = *cell;
    block** blocks = tetr->blocks;
    for (unsigned i = 0; i < 4; i++) {
        temp.x = cell->w * (blocks[i]->x +origoX) +offset_x;
        temp.y = cell->h * (blocks[i]->y +origoY) +offset_y;

        //  Determine if block is in game area
        if (ignorearea || temp.y >= offset_y) {
            if (texture) RenderSprite(ren, texture, tetr->blocks[0]->symbol, &temp);
            else SDL_RenderFillRect(ren, &temp);
        }
    }
}

int HandleEvents(UI_Functions* funs, SDL_Event* ev) {
    ui_sdl_data* data = (ui_sdl_data*)funs->data;

    switch (ev->type) {
        case SDL_QUIT: return 'q';
        case SDL_KEYDOWN: {
            switch (ev->key.keysym.scancode) {
                case SDL_SCANCODE_SPACE: return ' '; break;

                //  Movement
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_W: movementKeys |= 1 << INPUT_ROTATE; return 0;
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_A: movementKeys |= 1 << INPUT_LEFT; return 0;
                case SDL_SCANCODE_DOWN:
                case SDL_SCANCODE_S: movementKeys |= 1 << INPUT_DOWN; return 0;
                case SDL_SCANCODE_RIGHT:
                case SDL_SCANCODE_D: movementKeys |= 1 << INPUT_RIGHT; return 0;

                default: {
                    const char* key = SDL_GetKeyName(SDL_GetKeyFromScancode(ev->key.keysym.scancode));
                    //   All letters and numbers (excluding numpad-nums)
                    if (strlen(key) == 1) {
                        // printf("Button down: %c\n", key[0]);
                        return key[0];
                    }
                } break;
            }
        } break;
        case SDL_KEYUP: {
            switch (ev->key.keysym.scancode) {
                //  Movement
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_W: movementKeys &= ~(1 << INPUT_ROTATE); break;
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_A: movementKeys &= ~(1 << INPUT_LEFT); break;
                case SDL_SCANCODE_DOWN:
                case SDL_SCANCODE_S: movementKeys &= ~(1 << INPUT_DOWN); break;
                case SDL_SCANCODE_RIGHT:
                case SDL_SCANCODE_D: movementKeys &= ~(1 << INPUT_RIGHT); break;
                default: break;
            }
        } break;
        case SDL_WINDOWEVENT: {
            if (ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                AdjustCell(data->cell, ev->window.data1, ev->window.data2);
                return event_req_refresh;
            }
        } break;

        default: break;
    }
    return 0;
}

void RenderBox(SDL_Renderer* ren, SDL_Rect* cell, SpriteSheet* style, SDL_Rect* target) {
    SDL_Rect pos = *target;
    pos.w = cell->w;
    pos.h = cell->h;

    //  If no sprite sheet provided
    if (!style) {
        SDL_RenderDrawRect(ren, target);
        return;
    }
    //  background/filling
    for (; pos.y <= target->h; pos.y += pos.h) {
        for (pos.x = target->x; pos.x <= target->w; pos.x += pos.w) {
            RenderSprite(ren, style, 4, &pos);
        }
    }

    //  Vertical borders
    for (pos.y = target->y; pos.y <= target->h; pos.y += pos.h) {
        pos.x = target->x-cell->w;  //  left
        RenderSprite(ren, style, 3, &pos);

        pos.x = target->x+target->w; // right
        RenderSprite(ren, style, 5, &pos);
    }

    //  Horizontal borders
    for (pos.x = target->x; pos.x <= target->w; pos.x += pos.w) {
        pos.y = target->y-cell->h;  //  left
        RenderSprite(ren, style, 1, &pos);

        pos.y = target->y+target->h; // right
        RenderSprite(ren, style, 7, &pos);
    }

    //  Corner pieces
    pos.x = target->x-cell->w;  // Left-Top
    pos.y = target->y-cell->h;
    RenderSprite(ren, style, 0, &pos);
    pos.x = target->x+target->w; // Right-Top
    RenderSprite(ren, style, 2, &pos);
    pos.y = target->y+target->h; // Right-Bottom
    RenderSprite(ren, style, 8, &pos);
    pos.x = target->x-cell->w;  // Left-Bottom
    RenderSprite(ren, style, 6, &pos);
}

void RenderSprite(SDL_Renderer* ren, SpriteSheet* sheet, unsigned clip, SDL_Rect* pos_target) {
    SDL_RenderCopy(ren, sheet->texture, &sheet->clips[clip], pos_target);
}
