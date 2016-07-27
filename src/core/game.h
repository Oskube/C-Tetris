#include "game_randomisers.h"

/**
    \brief Enumerated type for different inputs which can be passed to game
*/
typedef enum {
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DOWN,     /**< Same as calling Update() */
    INPUT_ROTATE,   /**< Rotate clockwise */
    INPUT_SET       /**< Hard drop */
} player_input;

/**
    \brief Enumerated type for each different tetromino shapes
*/
typedef enum {
    SHAPE_O,
    SHAPE_I,
    SHAPE_T,
    SHAPE_L,
    SHAPE_J,
    SHAPE_S,
    SHAPE_Z,
    SHAPE_MAX
} tetromino_shape;

/**
    \brief A structure that contains the definition of a block

    Position x,y is only used when tetromino is active.
*/
typedef struct {
    unsigned symbol; /**< The symbol or color representing the block */
    int x; /**< Position relative to tetrominos origo */
    int y; /**< Position relative to tetrominos origo */
    // struct tetromino* owner; /* pointer to the owner tetromino struct */
} block; // polymino

/**
    \brief A container struct for active tetromino
*/
typedef struct {
    block** blocks; /**< Pointer to blocks which are part of tetromino */
    // unsigned count; /**< Keeps count of blocks alive, if 0 free block */
    tetromino_shape shape; /**< The shape of tetromino */
    unsigned x; /**< The origo of tetromino*/
    unsigned y; /**< The origo of tetromino*/
} tetromino;

/**
    \brief A structure that contains all map/matrix related data
*/
typedef struct {
    unsigned width;  /**< The width of the map */
    unsigned height; /**< The height of the map */
    block** blockMask;  /**< Array of pointers, map data */
} game_map;

/**
    \brief A structure that contains game events and other info.
*/
typedef struct {
    int ended;    /**< Tells if game has ended */
    unsigned score; /**< Player scores */
    unsigned rows;  /**< Number of rows destroyed */
    unsigned countTetromino[SHAPE_MAX]; /**< Count of each different tetromino spawned */

    unsigned level; /**< Current level */
    unsigned combo; /**< Current combo */
    int rowsToNextLevel; /**< Rows to clear until next level */

    int ghostY; /**< Ghost of the active tetromino */

    tetromino* next;     /**< The next tetromino */
    void* randomiser_data; /**< The data used by randomiser functions */
    unsigned (*fnRandomiserInit)(void*); /**< Function pointer to randomiser init */
    unsigned (*fnRandomiserNext)(void*); /**< Function pointer to randomiser next */
} game_info;

/**
    \brief A structure that contains everything what a game session needs.
*/
typedef struct {
    game_map   map;     /**< A matrix where tetrominos land */
    game_info  info;    /**< Game statistics */
    tetromino* active;  /**< A pointer to the active user controlled tetromino */

    unsigned nextUpdate; /**< Time of next update */
    unsigned step; /**< Time step between updates */
    unsigned (*fnMillis)(); /**< Function used to get current time in milliseconds */
} game;

/**
    \brief Initalize new game instance.
    \param width The width of new game area
    \param height The height of new game area
    \param randomiser Randomiser used in tetromino creation
    \param fntime Function pointer to a time function
    \return Pointer to new game instance

    \remark You must use FreeGame() to free allocated memory.
*/
extern game* Initialize(unsigned width, unsigned height, randomiser_type randomiser, unsigned (*fntime)());

/**
    \brief Process one step of game logic
    \param ptr Pointer to game instance
    \return Number of rows destroyed. If negative something else has happened.
*/
extern int Update(game* ptr);

/**
   \brief Processes given user input with game instance.
   \param ptr Pointer to game instance
   \param input Input from user
   \return 0 on success
*/
extern int ProcessInput(game* ptr, player_input input);

/**
    \brief Free memory allocated for the game.
    \param ptr Pointer to the game instance being freed
*/
extern void FreeGame(game* ptr);

/**
    \brief Reset given game
    \param ptr Pointer to game instance
*/
extern void ResetGame(game* ptr);
