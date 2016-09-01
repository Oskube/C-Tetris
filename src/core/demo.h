typedef struct {
    unsigned time; /**< Time from start in milliseconds when given */
    unsigned instruction; /**< The instruction */
} demo_instruction;

typedef struct List {
    void* value; /**< Pointer to the instruction */
    struct List* next;  /**< Pointer to the next list element */
} demo_list;

typedef struct {
    demo_list* instrsFirst; /**< First instruction int the list */
    demo_list* piecesFirst; /**< First piece in the list */
    demo_list* instrsCurrent; /**< Pointer to the current instruction */
    demo_list* piecesCurrent; /**< Pointer to the current piece */

    unsigned piecesCount; /**< Count of pieces*/
    unsigned instrsCount; /**< Count of instructions*/
} demo;

/**
    \brief Initializes a new demo instance
    \return Pointer to new demoinstance

    \note Use DemoFree() to delete instance
*/
extern demo* DemoCreateInstance(void);
/**
    \brief Frees memory allocated for demo instance
    \param ptr Pointer to the demo instance
*/
extern void DemoFree(demo* ptr);

/**
    \brief Adds an instruction to the list
    \param ptr Pointer to the demo instance
    \param instruction Instruction to add
    \return 0 on success
*/
extern int DemoAddInstruction(demo* ptr, unsigned time, unsigned instruction);
/**
    \brief Adds a piece to the list
    \param ptr Pointer to the demo instance
    \param shape Shape of the piece
    \return 0 on success
*/
extern int DemoAddPiece(demo* ptr, unsigned shape);

/**
    \brief Writes demo instance to a file
    \param ptr Pointer to the demo instance
    \param path Path to the file created
    \return Count of bytes written
*/
extern unsigned DemoSave(demo* ptr, const char* path);
/**
    \brief Reads file and creates demo instance
    \param path Path to file to read
    \return If success new demo instance, NULL on error

    \note Use DemoFree() to delete instance
*/
extern demo* DemoRead(const char* path);

typedef struct {demo_list* current;} demo_rand_data;

/**
    \brief Initializes tetromino queue for the demo playback
    \param demo Pointer to the demo instance
    \return The shape of the first tetromino
    \note Doesn't copy the demo struct or free given data
*/
extern void* DemoRandomizerInit(void* demo);

/**
    \brief Get the next tetromino recorded in demo
    \param Pointer to the list of tetrominos
    \return The shape of the next tetromino
    \note Modifies piecesCurrent pointer
*/
extern unsigned DemoRandomizerNext(void* data);
