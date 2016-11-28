#include "../ui.h"

/**
    \brief Initialize SDL UI

    Initializes SDL, creates renderer, window and assigns correct functions to given struct
    \param ret Pointer to struct used in returning function pointers
    \param argc Argument count
    \param argv Pointer to all program arguments
    \return 0 on success
*/
extern int UI_SDLInit(UI_Functions* ret, int argc, char** argv);

/**
    \brief Cleans up and closes SDL
*/
extern void UI_SDLCleanUp(UI_Functions* ptr);

/**
    \brief Get command line help for SDL UI
    \param Return pointer to help string
*/
extern const char* UI_SDLGetHelp();
