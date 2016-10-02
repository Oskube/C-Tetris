//  Header for all os dependent functions used by ncurses ui
//

/**
    \brief Get path to the executable
    \param buf Pointer to array where path is saved
    \param len Length of array
    \return Count of bytes placed in the buffer. On error -1 and buffer stays unchanged
*/
extern int GetExecutablePath(char* buf, unsigned len);

/**
    \brief Get time in current time in milliseconds.
    \return Current time in milliseconds
*/
extern unsigned GetTime();

/**
    \brief Sleeps for given time
    \param ms Time in milliseconds
*/
extern void SleepMs(unsigned ms);
