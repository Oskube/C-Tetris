#include <ctype.h> /* tolower() */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "states.h"

//  Static fsm functions
static int StateInit(UI_Functions* funs, void** data);
static void StateCleanUp(UI_Functions* funs);

//  Static vars used by this state
static bool is_running = false;
static game* gme = NULL;
static demo* record = NULL;
static unsigned start = 0; /* Time of the demo playback start, in ms */
static char* demoPath = NULL; /* Path of the demo */
static unsigned countInstruction = 0; /* Instruction count shown */
static demo_list* demoPosition = NULL; /* Pointer to the current demo instruction */

void* StatePlayDemo(UI_Functions* funs, void** data) {
    //  State init
    if (!is_running) {
        if (StateInit(funs, data) != 0) {
            if (demoPath) free(demoPath);
            return NULL;
        }
        is_running = true;
    }

    //  Process input
    int input = funs->UIGetInput(funs);
    switch (tolower(input)) {
        case 'q': is_running = false; break;
        default: break;
    }

    //  If demo hasn't ended
    if (demoPosition) {
        unsigned demoTime = funs->UIGetMillis() - start;  //  Calculate playback time
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
        char text[128];
        int len = snprintf(text, 128, "DEMO: %s\tInstruction: %u of %u", demoPath, countInstruction, record->instrsCount);
        if (countInstruction >= record->instrsCount && len < 128) {
            snprintf(text+len, 128-len, " - DEMO ENDED");
        }
        funs->UITextRender(funs, 0, 0, text);
    }
    Update(gme);
    funs->UIGameRender(funs, gme);

    //  If quit requested
    if (!is_running) {
        StateCleanUp(funs);
        return NULL;
    }
    return StatePlayDemo;
}

int StateInit(UI_Functions* funs, void** data) {
    if (!data) return -1;
    demoPath = *data;
    *data = NULL;
    if (!demoPath) return -2;

    if (funs->UIGameInit(funs)) {
        return -2;
    }

    //  Load demo file
    record = DemoRead(demoPath);
    if (!record) {
        fprintf(stderr, "Failed to load demo %s\n", demoPath);
        return -3;
    }

    //  Init demo game
    gme = InitDemoGame(MAP_WIDTH, MAP_HEIGHT+2, funs->UIGetMillis, record);

    demoPosition = record->instrsFirst; //  Set demo position to begining
    start = funs->UIGetMillis(); //  Set starting time of playback
    countInstruction = 0;

    return 0;
}

void StateCleanUp(UI_Functions* funs) {
    FreeGame(gme);
    gme = NULL;
    free(demoPath);
    demoPath = NULL;

    //  Free sub windows
    funs->UIGameCleanup(funs);
}
