#include <curses.h>

//  How many records are kept in the hiscore table
#define HISCORE_LENGTH 10
#define HISCORE_FILE "hiscores"

/*
    !States must free the additional data passed!
*/

typedef struct {
    unsigned randomiser;
} state_game_data;

/**
    \brief State function which handles gameplay

    Can take additional data as *state_game_data. When state ends it
    transitions to StateHiscores with additional data.
    \param win Pointer to ncurse WINDOW
    \param data Additional data used by state
    \return Function pointer to the next state
*/
extern void* StateGame(WINDOW* win, void** data);
/**
    \brief State function which handles displaying the high scores

    Handles displaying the high scores and adding new high scores in to the table.
    Can take in hiscore_list_entry* as additional data.
    \param win Pointer to ncurse WINDOW
    \param data Additional data used by state
    \return Function pointer to the next state
*/
extern void* StateHiscores(WINDOW* win, void** data);
/**
    \brief State function which handles playing recorded demo

    Additional data should have path to the demo. char* terminated by '\0'
    \param win Pointer to ncurse WINDOW
    \param data Additional data used by state
    \return Function pointer to the next state
*/
extern void* StatePlayDemo(WINDOW* win, void** data);
