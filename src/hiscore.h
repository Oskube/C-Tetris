/**
    \brief A container struct for a hi score table entry
*/
typedef struct {
    unsigned score; /**< Score */
    unsigned rows;  /**< Count of lines collapsed */
    unsigned lvl;   /**< Level reached */
    unsigned time;  /**< Game duration in milliseconds */
    unsigned date;  /**< Date of entry since the Epoch in seconds */
    char name[16];  /**< Player name or handle 15 character limit */
} hiscore_list_entry;

/**
    \brief Reads hi scores from a file
    \param file Path to hi score file
    \param ptrTable Pointer to array where scores will be read
    \param len Lenght of given array
    \return 0 on success
*/
extern int ReadHiScores(const char* file, hiscore_list_entry* ptrTable, unsigned len);

/**
    \brief Writes given hi score array to file
    \param file Path to hi score file
    \param ptrTable Pointer to array of scores to be saved
    \param len Lenght of given array
    \return the number of bytes written, <0 on errors
*/
extern int SaveHiScores(const char* file, hiscore_list_entry* ptrTable, unsigned len);

/**
    \brief Adds score to hi score list
    \param ptrTable Pointer to hi score table
    \param len Lenght of array
    \param ptrEntry Pointer to the new entry
    \return Ranking starting from 0
*/
extern int AddScoreToList(hiscore_list_entry* ptrTable, unsigned len, hiscore_list_entry *ptrEntry);

/**
    \brief Gets ranking with current score
    \param ptrTable Pointer to hi score table
    \param len Lenght of array
    \param score Score to be tested
    \return Ranking starting from 0
*/
extern int GetRanking(hiscore_list_entry* ptrTable, unsigned len, unsigned score);
