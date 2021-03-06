/*
    Header file for functions which are used to render and create ncurse ui for the game
*/

#include "../ui.h"

typedef struct {
    WINDOW* win;
    void* additional;
} curses_data;

/**
    \brief Inits game windows
    \param data Pointer to data used
    \return 0 on success
*/
extern int UI_CursesGameInit(UI_Functions* data);

/**
    \brief Deletes created windows
    \param data Pointer to data used
*/
extern void UI_CursesGameCleanup(UI_Functions* data);

/**
    \brief Updates game and renders it
    \param data Pointer to data used
    \param gme Pointer to the game instance used
*/
extern int UI_CursesGameRender(UI_Functions* data, game* gme);

/**
    \brief Draw high score table

    \param data Pointer to data used
    \param list Pointer to high score table data
    \param len Lenght of high score array
*/
extern void UI_CursesHiscoreRenderBegin(UI_Functions* data);

/**
    \brief Asks user name for high score table
    \param data Pointer to data used
    \param entry Pointer to high score entry
    \param rank Ranking of the new entry
*/
extern int UI_CursesHiscoreGetName(UI_Functions* data, hiscore_list_entry* entry, unsigned maxlen, unsigned rank);

/**
    \brief Draw given text to given position
    \param data Pointer to data used
    \param x column
    \param y row
    \param text Pointer to the text string
*/
extern void UI_CursesTextRender(UI_Functions* data, unsigned x, unsigned y, text_color color, char* text);

/**
    \brief Read keyboard
    \return Pressed key
*/
extern int UI_CursesGetInput(UI_Functions* funs);

/**
    \brief Sleeps for a while
    \param data Pointer to data used
*/
extern void UI_CursesMainLoopEnd(UI_Functions* data);

/**
    \brief Get executable path
    \param data Not used
    \param buf Buffer where path is copied
    \param len Maximum bytes copied
    \return Ammount of bytes copied
*/
extern int UI_CursesGetExePath(UI_Functions* data, char* buf, unsigned len);

/**
    \brief Draw tetromino
*/
extern void UI_CursesTetrominoRender(UI_Functions* data, unsigned topx, unsigned topy, tetromino* tetr);

/**
    \brief Init info area and returns position
    \param data Pointer to data used
    \param x    Pointer where top-x is returned
    \param y    Pointer where top-y is returned
*/
extern void UI_CursesBeginGameInfo(UI_Functions* data, unsigned* x, unsigned* y);

/**
    \brief Show pressed keys
    \param data Pointer to data
    \param topx Coordinate of left border
    \param topy Coordinate of top border
    \param instruction Current demo instructions
*/
extern void UI_CursesDemoShowPressed(UI_Functions* data, unsigned topx, unsigned topy, demo_instruction* instruction);

/**
    \brief Set tetromino symbols

    Takes max 7 characters as input. If less provided remaining characters won't be changed.
    \param newsyms String of new symbols
*/
extern void ChangeSymbols(char* newsyms);
