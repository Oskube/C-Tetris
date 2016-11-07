#include "common.h"

void ShowGameInfo(UI_Functions* funs, game* gme, unsigned showNext) {
    game_info* s = &(gme->info);
    char temp[256] = {0};

    unsigned topx = 0, topy = 0;
    funs->UIBeginGameInfo(funs, &topx, &topy);

    // Scores
    snprintf(temp, 256, "Level: %d (%d)", s->level, s->rowsToNextLevel);
    funs->UITextRender(funs, topx, topy++, color_red, temp);

    snprintf(temp, 256, "Lines: %d", s->rows);
    funs->UITextRender(funs, topx, topy++, color_green, temp);

    snprintf(temp, 256, "Score: %d", s->score);
    funs->UITextRender(funs, topx, topy++, color_green, temp);

    funs->UITextRender(funs, topx, topy++, color_green, "Next:");

    //  Next tetromino
    if (showNext) {
        funs->UITetrominoRender(funs, topx+3, topy, s->next);
        topy += 3;
    }

    //  Statistics for tetrominos
    unsigned total = 0;
    static const char minos[] = "OITLJSZ";
    static const text_color colors[] = {color_yellow, color_cyan, color_magenta, color_yellow, color_blue, color_green, color_red};

    funs->UITextRender(funs, topx, topy++, color_default, "Statistics:");
    for (unsigned i=0; i < 7; topy++, i++) {
        snprintf(temp, 256, "%c: %d", minos[i], s->countTetromino[i]);
        funs->UITextRender(funs, topx, topy, colors[i], temp);
        total += s->countTetromino[i];
    }
    snprintf(temp, 256, "Total: %d", total);
    funs->UITextRender(funs, topx, topy, color_default, temp);
}
