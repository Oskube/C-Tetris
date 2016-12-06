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

static void ShowHelp(UI_Functions* funs, unsigned x, unsigned y);
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

static bool showKeys = true;
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

    static unsigned infox = 0, infoy = 0;

    demo_instruction* inst = NULL;
    //  If demo hasn't ended
    if (demoPosition) {
        unsigned timeDelta = funs->UIGetMillis() - timeLast;  //  Calculate playback time
        timeLast = funs->UIGetMillis();
        timeDemo += timeScale*timeDelta;

        inst = (demo_instruction*)demoPosition->value; // Get current instruction

        //  Process instructions until we have to wait for the next one
        while (inst->time < timeDemo) {
            //  Send instruction to game instance
            if (inst->instruction == INPUT_UPDATE) {
                // Force game update
                gme->nextUpdate = 0;
                GameUpdate(gme);
            } else {
                if (showKeys) funs->UIDemoShowPressed(funs, infox+20, infoy+12, inst);
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
    ShowGameInfo(funs, gme, true, &infox, &infoy);

    if (showKeys) funs->UIDemoShowPressed(funs, infox+20, infoy+12, NULL); // Render empty grid if nothing happens

    ShowHelp(funs, infox+18, infoy+7);

    funs->UITextRender(funs, infox, infoy+17, color_red, infoName);
    funs->UITextRender(funs, infox, infoy+18, color_red, infoInstr);
    funs->UITextRender(funs, infox, infoy+19, color_red, infoTScal);
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

    //  Get demo path
    state_demo_data* settings = *data;
    demoPath = settings->path;
    if (!demoPath) return -2;

    showKeys = settings->showKeys;
    free(*data);
    *data = NULL;

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

void ShowHelp(UI_Functions* funs, unsigned x, unsigned y) {
    funs->UITextRender(funs, x, y++, color_white, "Controls:");
    funs->UITextRender(funs, x, y++, color_green, "LEFT, RIGHT - Change time scale");
    funs->UITextRender(funs, x, y++, color_green, "P           - Set time scale to 0");
    funs->UITextRender(funs, x, y, color_green, "Q           - QUIT");
}
