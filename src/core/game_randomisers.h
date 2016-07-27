/**
    \brief Enum for each different randomiser functions
*/
typedef enum {
    RANDOMISER_RANDOM,
    RANDOMISER_TGM,
    RANDOMISER_BAG,
    RANDOMISER_MAX
} randomiser_type;

/**
    \brief Structure that holds the data used by the bag randomiser functions
*/
typedef struct {
    unsigned tetrominos[7]; /**< Permutation of different shapes*/
    unsigned next; /**< Position of next tetromino */
} randombag;

/**
    \brief Bag randomiser initializer
    Initializes data and generates permutation
    \param bag Pointer to the randomiser data
    \return First shape in permutation, doesn't increase next counter
*/
extern unsigned RandomBagInit(void* bag);
/**
    \brief Bag randomiser processing
    \param bag Pointer to the randomiser data
    \return Shape of next tetromino
*/
extern unsigned RandomBagNext(void* bag);

/**
    \brief Structure for a TGM randomiser
*/
typedef struct {
    unsigned history[4];
    unsigned max_tries;
} randomiser_TGM_data;

/**
    \brief TGM randomiser initialize
    Fills history with OZSZ
    \param A pointer to the randomiser data
    \return Shape of tetromino
*/
extern unsigned RandomTGMInit(void* data);
/**
    \brief Get next tetromino
    \param A pointer to the randomiser data
    \return Shape of next tetromino
*/
extern unsigned RandomTGMNext(void* data);

/**
    \brief Get next random tetromino
    \param data should be NULL
    \return Shape of next tetromino
*/
extern unsigned RandomRandomInit(void* data);
/**
    \brief Get next random tetromino
    \param data should be NULL
    \return Shape of next tetromino
*/
extern unsigned RandomRandomNext(void* data);
