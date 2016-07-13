/**
    \brief Enum for each different randomiser functions
*/
typedef enum {
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
