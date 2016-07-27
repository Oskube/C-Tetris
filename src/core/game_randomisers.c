#include <stdlib.h>
#include "game_randomisers.h"

unsigned RandomBagNext(void* bag) {
    if (bag == NULL) return 0;
    randombag* b = (randombag*) bag;

    //  If bag emptied generate a new permutation
    if (++(b->next) >= 7) {
        RandomBagInit(b);
    }
    return b->tetrominos[b->next];
}

unsigned RandomBagInit(void* bag) {
    if (bag == NULL) return 0;
    randombag* b = (randombag*) bag;
    b->next = 0;

    unsigned i = 0;
    //  All tetrominos ordered in the bag
    for (i=0; i<7; i++) {
        b->tetrominos[i] = i;
    }
    //  Swap tetrominos randomly for the each index
    for (i=0; i <7; i++) {
        unsigned tmp = b->tetrominos[i];
        unsigned other = rand()%7;
        b->tetrominos[i] = b->tetrominos[other];
        b->tetrominos[other] = tmp;
    }
    return b->tetrominos[b->next];
}

unsigned RandomTGMInit(void* data) {
    if (data == NULL) return 0;
    randomiser_TGM_data* d = (randomiser_TGM_data*) data;

    d->history[0] = 0; // O
    d->history[1] = 6; // Z
    d->history[2] = 5; // S
    d->history[3] = 6; // Z

    d->max_tries = 4;
    return RandomTGMNext(data);
}

unsigned RandomTGMNext(void* data) {
    if (data == NULL) return 0;
    randomiser_TGM_data* d = (randomiser_TGM_data*) data;

    unsigned ret = 0;
    unsigned quit = 0;
    //  Try get tetromino which isn't in history
    for (unsigned i=0; i < d->max_tries || !quit; i++) {
        ret = rand()%7;
        quit = 1;
        //  Check history
        for (unsigned j=0; j<4; j++) {
            if (d->history[j] == ret) {
                quit=0;
                break;
            }
        }
    }

    //  Shift history and assign the new tetromino in to end of it
    for (unsigned i=0; i<3; i++) {
        d->history[i] = d->history[i+1];
    }
    d->history[3] = ret;

    return ret;
}

unsigned RandomRandomInit(void* data) {
    return RandomRandomNext(data);
}

unsigned RandomRandomNext(void* data) {
    return rand()%7;
}
