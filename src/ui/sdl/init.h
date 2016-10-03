#include "../ui.h"

/**
    \brief Initialize SDL UI

    Initializes SDL, creates renderer, window and assigns correct functions to given struct
    \param ret Pointer to struct used in returning function pointers
    \return 0 on success
*/
extern int UI_SDLInit(UI_Functions* ret);

/**
    \brief Cleans up and closes SDL
*/
extern void UI_SDLCleanUp(UI_Functions* ptr);
