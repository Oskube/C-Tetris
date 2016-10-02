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
extern int GameWindowsInit(UI_Functions* data);

/**
    \brief Deletes created windows
    \param data Pointer to data used
*/
extern void GameWindowsFree(UI_Functions* data);

/**
    \brief Updates game and renders it
    \param data Pointer to data used
    \param gme Pointer to the game instance used
*/
extern int GameRender(UI_Functions* data, game* gme);

/**
    \brief Draw high score table

    \param data Pointer to data used
    \param list Pointer to high score table data
    \param len Lenght of high score array
*/
extern void DrawHiscores(UI_Functions* data, hiscore_list_entry* list, unsigned len);

/**
    \brief Asks user name for high score table
    \param data Pointer to data used
    \param entry Pointer to high score entry
    \param rank Ranking of the new entry
*/
extern void GetNewHiscoreName(UI_Functions* data, hiscore_list_entry* entry, unsigned maxlen, unsigned rank);

/**
    \brief Draw given text to given position
    \param data Pointer to data used
    \param x column
    \param y row
    \param text Pointer to the text string
*/
extern void DrawText(UI_Functions* data, unsigned x, unsigned y, char* text);

/**
    \brief Read keyboard
    \return Pressed key
*/
extern int ReadKey();

/**
    \brief Sleeps for a while
*/
extern void LoopEnd(UI_Functions* data);
