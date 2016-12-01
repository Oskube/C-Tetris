#include <ctype.h> /* tolower() */
#include <string.h> /* strlen() */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "states.h"
#include "common.h"

#define INFO_LEN 64

//  Static fsm functions
static int StateInit(UI_Functions* funs, void** data);
static void StateCleanUp(UI_Functions* funs);

/**
    \brief Wrapper function to return demo time
*/
static unsigned GetDemoTime();

//  Static vars used by this state
static bool is_running = false;
static game* gme = NULL;
static demo* record = NULL;
static unsigned start = 0; /* Time of the demo playback start, in ms */
static char* demoPath = NULL; /* Path of the demo */
static unsigned countInstruction = 0; /* Instruction count shown */
static demo_list* demoPosition = NULL; /* Pointer to the current demo instruction */

static unsigned timeLast = 0;
static float timeDemo = 0;
static float timeScale = 1; /* Used in fast forwarding */

static char infoName[INFO_LEN];
static char infoInstr[INFO_LEN];
static char infoTScal[INFO_LEN]; // time scale

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
    unsigned icount = funs->UIGetInput(funs); // Fill input array
    for (unsigned iii = 0; iii < icount; iii++) { // Process all inputs
        switch (tolower(funs->inputs[iii])) {
            case 'q': is_running = false; break;
            case 'a': {
                timeScale -= 0.1f;
                snprintf(infoTScal, INFO_LEN, "Time scale: %.2f", timeScale);
            } break;
            case 'd': {
                timeScale += 0.1f;
                snprintf(infoTScal, INFO_LEN, "Time scale: %.2f", timeScale);
            } break;
            case 'p': {
                timeScale = 0;
                snprintf(infoTScal, INFO_LEN, "Time scale: PAUSED");
            } break;
            default: break;
        }
    }
    if (timeScale < 0) {
        timeScale = 0;
        snprintf(infoTScal, INFO_LEN, "Time scale: PAUSED");
    }

    //  If demo hasn't ended
    if (demoPosition) {
        // unsigned demoTime = funs->UIGetMillis() - start;  //  Calculate playback time
        unsigned timeDelta = funs->UIGetMillis() - timeLast;  //  Calculate playback time
        timeLast = funs->UIGetMillis();
        timeDemo += timeScale*timeDelta;

        demo_instruction* inst = (demo_instruction*)demoPosition->value; // Get current instruction

        //  Process instructions until we have to wait for the next one
        while (inst->time < timeDemo) {
            //  Send instruction to game instance
            if (inst->instruction == INPUT_UPDATE) {
                // Force game update
                gme->nextUpdate = 0;
                GameUpdate(gme);
            } else {
                GameProcessInput(gme, (player_input)inst->instruction);
            }

            //  Proceed to next instruction
            demoPosition = demoPosition->next;
            countInstruction++;
            if (!demoPosition) break; // end of demo

            inst = (demo_instruction*)demoPosition->value;
        }

        //  Generate info texts
        int len = snprintf(infoInstr, INFO_LEN, "Instruction: %u of %u", countInstruction, record->instrsCount);
        if (countInstruction >= record->instrsCount && len < INFO_LEN) {
            snprintf(infoInstr+len, INFO_LEN-len, " - DEMO ENDED");
        }
    }

    // Renderings
    ShowGameInfo(funs, gme, true);
    funs->UITextRender(funs, 30, 18, color_red, infoName);
    funs->UITextRender(funs, 30, 19, color_red, infoInstr);
    funs->UITextRender(funs, 30, 20, color_red, infoTScal);
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
    gme = GameInitDemo(MAP_WIDTH, MAP_HEIGHT+2, GetDemoTime, record);

    demoPosition = record->instrsFirst; //  Set demo position to begining
    timeLast = start = funs->UIGetMillis(); //  Set starting time of playback
    countInstruction = 0;
    timeScale = 1;
    snprintf(infoName, INFO_LEN, "DEMO: %s", demoPath);
    snprintf(infoTScal, INFO_LEN, "Time scale: %.2f", timeScale);

    return 0;
}

void StateCleanUp(UI_Functions* funs) {
    GameFree(gme);
    gme = NULL;
    free(demoPath);
    demoPath = NULL;

    //  Free sub windows
    funs->UIGameCleanup(funs);
}

unsigned GetDemoTime() {
    return timeDemo;
}
