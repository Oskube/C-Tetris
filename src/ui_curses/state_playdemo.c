#include <ctype.h> /* tolower() */
#include <stdio.h>
#include <stdlib.h>

#include "states.h"
#include "game_functions.h"
#include "os_dependent.h"

//  Static fsm functions
static int StateInit(WINDOW* win, void** data);
static void StateCleanUp();

static void PrintDemoInfo(); // Prints demo path and instruction count

//  Static vars used by this state
static bool is_running = false;
static ncurse_game_windows windows = {.map = NULL, .info = NULL};
static game* gme = NULL;
static demo* record = NULL;
static unsigned start = 0; /* Time of the demo playback start, in ms */
static char* demoPath = NULL; /* Path of the demo */
static unsigned countInstruction = 0; /* Instruction count shown */
static demo_list* demoPosition = NULL; /* Pointer to the current demo instruction */

void* StatePlayDemo(WINDOW* win, void** data) {
    //  State init
    if (!is_running) {
        if (StateInit(win, data) != 0) {
            if (demoPath) free(demoPath);
            return NULL;
        }
        is_running = true;
    }

    //  Process input
    int input = getch();
    switch (tolower(input)) {
        case 'q': is_running = false; break;
        default: break;
    }

    //  If demo hasn't ended
    if (demoPosition) {
        unsigned demoTime = GetTime() - start;  //  Calculate playback time
        demo_instruction* inst = (demo_instruction*)demoPosition->value; // Get current instruction

        //  Process instructions until we have to wait for the next one
        while (inst->time < demoTime) {
            //  Send instruction to game instance
            ProcessInput(gme, (player_input)inst->instruction);
            //  Proceed to next instruction
            demoPosition = demoPosition->next;
            countInstruction++;
            if (!demoPosition) break; // end of demo

            inst = (demo_instruction*)demoPosition->value;
        }
        PrintDemoInfo();
    }
    UpdateAndRender(gme, &windows);

    //  If quit requested
    if (!is_running) {
        StateCleanUp();
        return NULL;
    }
    return StatePlayDemo;
}

int StateInit(WINDOW* win, void** data) {
    if (!data) return -1;
    demoPath = *data;
    *data = NULL;
    if (!demoPath) return -2;

    if (GameWindowsInit(win, &windows)) {
        return -2;
    }

    //  Load demo file
    record = DemoRead(demoPath);
    if (!record) {
        fprintf(stderr, "Failed to load demo %s\n", demoPath);
        return -3;
    }

    //  Init demo game
    gme = InitDemoGame(MAP_WIDTH, MAP_HEIGHT+2, GetTime, record);

    demoPosition = record->instrsFirst; //  Set demo position to begining
    start = GetTime(); //  Set starting time of playback
    countInstruction = 0;

    return 0;
}

void StateCleanUp() {
    FreeGame(gme);
    gme = NULL;
    free(demoPath);
    demoPath = NULL;

    //  Free sub windows
    GameWindowsFree(&windows);
}

void PrintDemoInfo() {
    mvprintw(0, 0, "DEMO: %s\tInstruction: %u of %u", demoPath, countInstruction, record->instrsCount);
    if(!demoPosition) printw(" - DEMO ENDED");
}
