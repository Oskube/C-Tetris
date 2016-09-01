/*
    Header file for functions which are used to render and create ncurse ui for game
*/

#include <curses.h>
#include "../core/game.h"

#define MAP_WIDTH  10
#define MAP_HEIGHT 20

/**
    \brief Struct for windows used by game functions
*/
typedef struct {
    WINDOW* map; /**< Window for game area or map*/
    WINDOW* info; /**< Window for game info */
} ncurse_game_windows;

/**
    \brief Inits game windows
    \param win Pointer to main window
    \param ret Pointer to struct used in returning created windows
    \return 0 on success
*/
extern int GameWindowsInit(WINDOW* win, ncurse_game_windows* ret);

/**
    \brief Deletes created windows
    \pararm windows Pointer to the struct

    \note Doesn't free struct passed!
*/
extern void GameWindowsFree(ncurse_game_windows* windows);

/**
    \brief Updates game and renders it
    \param gme Pointer to the game instance used
    \param windows Pointer to windows used
    \return Returns value from Update()
*/
extern int UpdateAndRender(game* gme, ncurse_game_windows* windows);

/**
    \brief Renders the current state of the game
    \param src Pointer to the game instance
    \param wins Pointer to the windows used in rendering
    \param showghost Determines if the ghost is shown
*/
extern void DrawMap(game* src, ncurse_game_windows* wins, bool showghost);

/**
    \brief Renders game info

    Shows current score and count of spawned pieces.
    \param src Pointer to the game instance
    \param wins Pointer to the windows used in rendering
*/
extern void DrawInfo(game* src, ncurse_game_windows* wins);
