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
