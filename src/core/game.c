#include <stdlib.h>
#include <stdbool.h>

#include "game.h"

#define MIN_DELAY_LEVEL 10
#define MAX_DELAY 1200
#define MIN_DELAY 150

static bool SetRandomiser(game* ptr, randomiser_type new_randomiser);

static bool ActiveCollided(game* ptr); // Test if collided with borders or other blocks
static void FreezeActive(game* ptr); // !_Frees active tetromino partly_!
static int  ClearFilledRows(game* ptr, unsigned y, unsigned h);
static bool IsRowFull(game_map* ptr, unsigned row);
static bool ClearAndCollapse(game_map* ptr, unsigned row);

//  For active tetromino
static tetromino* TetrominoNew(tetromino_shape shape, unsigned x);
static int TetrominoMove(game* ptr, player_input dir);
static int TetrominoRotate(game* ptr);
static int TetrominoRotateKick(game* ptr);
static void TetrominoFree(tetromino* ptr);
static int CalcGhost(game* ptr);
static void HardDrop(game* ptr);

game* Initialize(unsigned width, unsigned height, randomiser_type randomiser, unsigned (*fntime)()) {
    if (width == 0 || height == 0 || fntime == NULL) return NULL;
    game* ptrGame = (game*)malloc(sizeof(game));
    if (!ptrGame) return NULL;

    //  Set map dimensions
    ptrGame->map.width = width;
    ptrGame->map.height = height;

    //  Set pointers to NULL.
    ptrGame->active = NULL;
    ptrGame->info.next = NULL;
    ptrGame->info.randomiser_data = NULL;
    ptrGame->info.fnRandomiserInit = NULL;
    ptrGame->info.fnRandomiserNext = NULL;
    ptrGame->fnMillis = fntime;
    ptrGame->demorecord = NULL;

    //  Allocate memory for blockmask and initialize it 0
    ptrGame->map.blockMask = (block**)calloc(width*height, sizeof(block*) * width*height);
    if (!ptrGame->map.blockMask) {
        FreeGame(ptrGame);
        return NULL;
    }

    //  Randomiser setup
    if(!SetRandomiser(ptrGame, randomiser)) {
        FreeGame(ptrGame);
        return NULL;
    }

    ResetGame(ptrGame);
    return ptrGame;
}

int Update(game* ptr) {
    if (!ptr) return -1;
    if (ptr->info.ended) return -2;

    unsigned milliseconds = ptr->fnMillis();
    int ret = 0;

    //  Check if the timer has expired.
    if (ptr->nextUpdate - milliseconds <= ptr->step) return ret;

    //  Check if active tetromino hit bottom or tetromino below.
    if (TetrominoMove(ptr, INPUT_DOWN)) {
        int origoy = ptr->active->y;
        FreezeActive(ptr);

        //  Check rows, there can be blocks above origo
        if (origoy < 2) {
            origoy = 0;
        } else {
            origoy -= 2;
        }

        ret = ClearFilledRows(ptr, origoy, 5);

        game_info* s = &ptr->info;
        if (ret > 0) {
            s->score += ret*50*(s->level+1) + (s->combo*10*(s->level+1));
            s->combo++;

            //  Add rows to counter and to level progress
            s->rows += ret;
            s->rowsToNextLevel -= ret;
            bool levels = false;
            while (s->rowsToNextLevel <= 0) {
                s->level += 1;
                s->rowsToNextLevel += (s->level)*3;
                levels = true;
            }

            //  If level has changed calculate new step duration
            if (levels) {
                unsigned lvl = s->level;
                if (lvl > MIN_DELAY_LEVEL) lvl = MIN_DELAY_LEVEL;
                ptr->step = (MAX_DELAY-MIN_DELAY)*(MIN_DELAY_LEVEL-lvl)/MIN_DELAY_LEVEL + MIN_DELAY;
            }
        } else {
            s->combo = 0;
        }

        //  Assign next to active tetromino
        ptr->active = s->next;
        s->countTetromino[ptr->active->shape] += 1;
        //  Create a new next tetromino
        tetromino_shape shape = s->fnRandomiserNext(s->randomiser_data);
        s->next = TetrominoNew(shape, ptr->map.width/2);
        //  Add it to the demo record
        DemoAddPiece(ptr->demorecord, s->next->shape);

        //  Calculate ghost for the new active tetromino
        CalcGhost(ptr);

        if (ActiveCollided(ptr)) {
            s->ended = 1;
            return -2; //   New tetromino already collided with something -> game over
        }
    }

    //  Set time of next update
    ptr->nextUpdate = milliseconds + ptr->step;

    return ret;
}

int ProcessInput(game* ptr, player_input input) {
    if (!ptr) return -1;
    if (ptr->info.ended) return -2;

    tetromino* act = ptr->active;
    if (!act) return 0;

    //  Add input to the demo record
    DemoAddInstruction(ptr->demorecord, ptr->fnMillis() - ptr->info.timeStarted, (unsigned int)input);

    int ret = 0;
    switch (input) {
        case INPUT_LEFT:
        case INPUT_RIGHT: {
            ret = TetrominoMove(ptr, input);
        } break;
        case INPUT_DOWN: ptr->nextUpdate = 0; break;
        case INPUT_ROTATE: {
            ret = TetrominoRotateKick(ptr);
        } break;
        case INPUT_SET: HardDrop(ptr); break;
        default: break;
    }
    return ret;
}

void FreeGame(game* ptr) {
    if (!ptr) return;

    //  Free randomiser data
    if (ptr->info.randomiser_data) free(ptr->info.randomiser_data);
    //  Free blockmask and active
    if (ptr->active) TetrominoFree(ptr->active);
    //  Free next tetromino
    if (ptr->info.next) TetrominoFree(ptr->info.next);

    //  Free map
    unsigned len = ptr->map.width * ptr->map.height;
    for (unsigned i = 0; i < len; i++) {
        block* temp = ptr->map.blockMask[i];
        if(temp) {
            free(temp);
            temp = NULL;
        }
    }
    free(ptr->map.blockMask);

    //  Free recorded demo
    DemoSave(ptr->demorecord, "demo.dm");
    DemoFree(ptr->demorecord);
    ptr->demorecord = NULL;

    //  Free game struct
    free(ptr);
}

void ResetGame(game* ptr) {
    if (ptr == NULL) return;
    //  Reset map
    unsigned len = ptr->map.width * ptr->map.height;
    for (unsigned i = 0; i < len; i++) {
        block* temp = ptr->map.blockMask[i];
        if(temp) {
            free(temp);
            temp = NULL;
        }
    }

    // Reset stats
    game_info* s = &(ptr->info);
    s->score  = 0;
    s->rows   = 0;
    s->level  = 0;
    s->combo  = 0;
    s->ended  = 0;
    s->rowsToNextLevel  = 2;
    s->timeStarted = ptr->fnMillis();

    for (unsigned i=0;i<SHAPE_MAX;i++) s->countTetromino[i] = 0;

    //  Update timer
    ptr->step = MAX_DELAY;
    ptr->nextUpdate = ptr->fnMillis() + ptr->step;

    //  Demo record init
    DemoFree(ptr->demorecord);
    ptr->demorecord = NULL;
    ptr->demorecord = DemoCreateInstance();

    // Free active tetromino
    if (ptr->active) TetrominoFree(ptr->active);

    //  Free next tetromino
    if (s->next) TetrominoFree(s->next);

    //  Create new randoms
    tetromino_shape shape = s->fnRandomiserInit(s->randomiser_data);
    //  Create first and next tetromino
    ptr->active = TetrominoNew(shape, ptr->map.width/2);
    s->countTetromino[ptr->active->shape] += 1;

    shape = s->fnRandomiserNext(s->randomiser_data);
    s->next = TetrominoNew(shape, ptr->map.width/2);

    //  record first 2 shapes to demo
    DemoAddPiece(ptr->demorecord, ptr->active->shape);
    DemoAddPiece(ptr->demorecord, s->next->shape);

    //  Calculate ghost
    CalcGhost(ptr);
}

unsigned GetGameTime(game* ptr) {
    if (ptr==NULL) return 0;

    return ptr->fnMillis() - (ptr->info.timeStarted);
}

/*
    Static functions
*/

/**
    \brief Function to set randomiser
    \param ptr Pointer to the game instance
    \param new_randomiser Randomiser to set
    \return True on success
*/
bool SetRandomiser(game* ptr, randomiser_type new_randomiser) {
    if (!ptr) return false;

    //  Free old randomiser data
    game_info* nfo = &(ptr->info);
    if (nfo->randomiser_data) free(nfo->randomiser_data);

    switch (new_randomiser) {
        case RANDOMISER_BAG: {
            nfo->randomiser_data = malloc(sizeof(randombag));
            if (!nfo->randomiser_data) return false;
            nfo->fnRandomiserInit = &RandomBagInit;
            nfo->fnRandomiserNext = &RandomBagNext;
        } break;
        case RANDOMISER_TGM: {
            nfo->randomiser_data = malloc(sizeof(randomiser_TGM_data));
            if (!nfo->randomiser_data) return false;
            nfo->fnRandomiserInit = &RandomTGMInit;
            nfo->fnRandomiserNext = &RandomTGMNext;
        } break;
        case RANDOMISER_RANDOM:
        default: {
            nfo->randomiser_data = NULL;
            nfo->fnRandomiserInit = &RandomRandomInit;
            nfo->fnRandomiserNext = &RandomRandomNext;
        } break;
    }
    return true;
}

/**
    \brief Free memory allocated by TetrominoNew()
    \param ptr Pointer to the tetromino being freed
*/
void TetrominoFree(tetromino* ptr) {
    if (ptr) {
        for (int i = 0; i < 4; i++) {
            if (ptr->blocks[i]) {   //  Free all blocks
                free(ptr->blocks[i]);
                ptr->blocks[i] = NULL;
            }
        }
        free(ptr->blocks);  // Free allocated pointer array
        free(ptr);  //  Free tetromino
    }
}

/**
    \brief Check if active tetromino has collided with something.
    \param ptr Pointer to game object
    \return If collided with something true. Otherwise false
*/
bool ActiveCollided(game* ptr) {
    tetromino* act = ptr->active;

    //  If active tetromino exists
    if (act) {
        unsigned w = ptr->map.width,
                 h = ptr->map.height;

        //  Check collisions for every block of tetromino.
        unsigned tx = act->x,
                 ty = act->y;
        for (int i = 0; i < 4; i++) {
            block* cur = act->blocks[i];
            unsigned x = cur->x+tx;
            unsigned y = cur->y+ty;
            if (x >= w) return true; // To left or right border
            if (y >= h) return true; // To bottom or top

            //  Check collision to other blocks
            unsigned pos = y*w + x;
            if (ptr->map.blockMask[pos]) return true;
        }
     }
     //  If no collision
     return false;
}

/**
    \brief Updates active tetromino to game map
    \param ptr Pointer to game

    \warning Doesn't free allocated blocks. Only the container struct. Use TetrominoFree() to free the whole tetromino and its blocks
*/
void FreezeActive(game* ptr) {
    tetromino* t = ptr->active;
    for (unsigned i = 0; i < 4; i++) {
        unsigned pos = (t->y+t->blocks[i]->y)* ptr->map.width + t->blocks[i]->x + t->x;
        ptr->map.blockMask[pos] = t->blocks[i];
    }

    //  Free active tetromino
    free(t->blocks); // free allocated array
    free(t);
    ptr->active = NULL;
}

/**
    \brief Clears filled rows
    \param ptr Pointer to game instance
    \param y Row where to start
    \param h How many rows are checked
    \return Count of cleared rows
*/
int ClearFilledRows(game* ptr, unsigned y, unsigned h) {
    unsigned count = 0;
    unsigned pos = y+h;
    if (pos > ptr->map.height) pos = ptr->map.height;

    for (unsigned i = 0; i < h; i++) {
        if (IsRowFull(&(ptr->map), pos)) {
            ClearAndCollapse(&(ptr->map), pos);
            count++;
        } else if (pos > 0) {
            pos--;
        }
    }
    return count;
}

/**
    \brief Checks given row
    \param ptr Pointer to map
    \param row Row to CheckRow
    \return True if row is complete
*/
bool IsRowFull(game_map* ptr, unsigned row) {
    if (ptr == NULL) return false;

    unsigned pos = row*ptr->width;
    for (unsigned i = 0; i < ptr->width; i++) {
        if (!ptr->blockMask[pos+i]) return false;
    }
    return true;
}

/**
    \brief Clear given row and drop above
    \param ptr Pointer to map
    \param row Row to clear
    \return True if rows were dropped
*/
bool ClearAndCollapse(game_map* ptr, unsigned row) {
    if (ptr == NULL) return false;

    //  Clear row
    bool collapsed = false;
    unsigned pos = row*ptr->width;
    for (unsigned i = 0; i < ptr->width; i++) {
        if (ptr->blockMask[pos+i]) {
            free(ptr->blockMask[pos+i]);    // Free blocks
            ptr->blockMask[pos+i] = NULL;   // Pointer to null
        }
    }

    //  Drop lines above 1 block down
    while (row > 0) {
        row--;
        bool line = false;
        unsigned pos = row*ptr->width;
        for (unsigned i = 0; i < ptr->width; i++) {
            if (ptr->blockMask[pos+i]) {
                ptr->blockMask[pos+i+ptr->width] = ptr->blockMask[pos+i];
                ptr->blockMask[pos+i] = NULL;
                line = true;
            }
        }
        //  If no line was dropped break from loop
        if (line == false) break;
        else collapsed = true;
    }

    return collapsed;
}

/**
    \brief Move active tetromino to given direction. If block collides with something move tetromino back to its original location.
    \param ptr Pointer to the game instance
    \param dir Direction where to move
*/
int TetrominoMove(game* ptr, player_input dir) {
    int d = 1;

    if (dir == INPUT_DOWN) {    //  If we want to move it down
        //  Move tetromino
        ptr->active->y += d;

        //  Revert move if collided and return 1
        if (ActiveCollided(ptr)) {
            d = -d;
            ptr->active->y += d;
            return 1;
        }
    } else {
        if (dir == INPUT_LEFT) d = -1;
        //  Move tetromino
        ptr->active->x += d;

        //  Revert move if collided and return 1
        if (ActiveCollided(ptr)) {
            d = -d;
            ptr->active->x += d;
            return 1;
        }

        //  Recalculate ghost
        CalcGhost(ptr);
    }
    return 0;
}

/**
    \brief Rotate tetromino clockwise
    \param Pointer to game instance
    \return 0 on success
*/
int TetrominoRotate(game* ptr) {
    if (ptr->active->shape == SHAPE_O) {
        return 0;
    } else {
        //  Rotate clockwise
        for (unsigned i = 0; i < 4; i++) {
            block* cur = ptr->active->blocks[i];
            unsigned tmp = cur->x;
            cur->x = -cur->y;
            cur->y = tmp;
        }
        if (ActiveCollided(ptr)) {
            //  Rotate anti-clockwise
            for (unsigned i = 0; i < 4; i++) {
                block* cur = ptr->active->blocks[i];
                unsigned tmp = -cur->x;
                cur->x = cur->y;
                cur->y = tmp;
            }
            return 1;
        }
    }

    //  Recalculate ghost
    CalcGhost(ptr);
    return 0;
}

/**
    \brief Rotates clock wise with a wall kick
    \param Pointer to game instance
    \return 0 on success
*/
int TetrominoRotateKick(game* ptr) {
    int ret = 0;
    ret = TetrominoRotate(ptr);
    if (ret == 0) return ret;

    unsigned origX = ptr->active->x;

    //  Wall kicking
    //  Move right and try rotate
    ptr->active->x = origX+1;
    ret = TetrominoRotate(ptr);
    if (ret == 0) return ret; // Return if success

    //  Move left and try rotate
    ptr->active->x = origX-1;
    ret = TetrominoRotate(ptr);
    if (ret == 0) return ret;

    //  The Shape I is a special case
    if (ptr->active->shape == SHAPE_I) {
        ptr->active->x = origX+2;
        ret = TetrominoRotate(ptr);
        if (ret == 0) return ret;

        ptr->active->x = origX-2;
        ret = TetrominoRotate(ptr);
        if (ret == 0) return ret;
    }

    //  All tried rotations failed -> restore original position
    ptr->active->x = origX;
    return ret;
}

/**
    \brief Allocates and initializes new tetromino
    \param shape Shape of the new tetromino
    \param x Position in game where placed
    \return Pointer to new tetromino
*/
tetromino* TetrominoNew(tetromino_shape shape, unsigned x) {
    tetromino* ret = (tetromino*)malloc(sizeof(tetromino));
    if (!ret) return NULL;

    ret->blocks = (block**)malloc(sizeof(block*)*4);
    if (!ret->blocks) {
        free(ret);
        return NULL;
    }

    ret->x = x-1;
    ret->y = 2; //  2 top rows are hidden
    ret->shape = shape;
    // ret->count = 0;

    for (int i = 0; i < 4; i++) {
        ret->blocks[i] = (block*)malloc(sizeof(block));
        ret->blocks[i]->x = 0;
        ret->blocks[i]->y = 0;
        ret->blocks[i]->symbol = shape;
    }

    //  Order blocks according to shape
    switch (shape) {
        case SHAPE_O: {
            ret->blocks[1]->x = 1;
            ret->blocks[2]->y = 1;
            ret->blocks[3]->x = 1;
            ret->blocks[3]->y = 1;
            ret->y -= 1;
        } break;
        case SHAPE_I: {
            ret->blocks[1]->x = -1;
            ret->blocks[2]->x = 1;
            ret->blocks[3]->x = 2;
        } break;
        case SHAPE_T: {
            ret->blocks[1]->y = -1;
            ret->blocks[2]->x = 1;
            ret->blocks[3]->x = -1;
        } break;
        case SHAPE_L: {
            ret->blocks[1]->x = -1;
            ret->blocks[2]->x = 1;
            ret->blocks[3]->x = 1;
            ret->blocks[3]->y = -1;
        } break;
        case SHAPE_J: {
            ret->blocks[1]->x = -1;
            ret->blocks[2]->x = 1;
            ret->blocks[3]->x = -1;
            ret->blocks[3]->y = -1;
        } break;
        case SHAPE_S: {
            ret->blocks[1]->x = -1;
            ret->blocks[2]->y = -1;
            ret->blocks[3]->x = 1;
            ret->blocks[3]->y = -1;
        } break;
        case SHAPE_Z: {
            ret->blocks[1]->x = 1;
            ret->blocks[2]->y = -1;
            ret->blocks[3]->x = -1;
            ret->blocks[3]->y = -1;
        } break;
        default: break; // Shouldn't happen
    }

    return ret;
}

/**
    \brief Calculates position of ghost of the active tetromino
    \param ptr Pointer to game instance
    \return y of the ghots
*/
int CalcGhost(game* ptr) {
    tetromino* tetr = ptr->active;
    unsigned origY = tetr->y;
    ptr->info.ghostY = tetr->y;

    bool moved = false;
    while (!ActiveCollided(ptr)) {
        tetr->y++;
        moved = true;
    }

    if (moved) {
        ptr->info.ghostY = tetr->y-1; // Set y of the collided tetromino to ghosty
    }
    tetr->y = origY; //  Restore original y of the active tetromino
    return ptr->info.ghostY;
}

/**
    \brief Drops active tetromino of given game instance to the place of ghost
    \param ptr Pointer to the game instance
*/
void HardDrop(game* ptr) {
    //  Make sure position of the ghost is correct
    int y = CalcGhost(ptr);
    ptr->active->y = y;

    // Call Update() to lock tetromino and generate new
    ptr->nextUpdate = 0;
}
