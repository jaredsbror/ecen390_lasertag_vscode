/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "trigger.h"

#define DEBUG_TRIGGER true  // If true, debug messages enabled

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).

// All printed messages for states are provided here.
#define INIT_ST_MSG "init state\n"
#define BBBB_ST_MSG "\n"
#define CCCC_ST_MSG "\n"
#define DDDD_ST_MSG "\n"
#define EEEE_ST_MSG "\n"
#define FFFF_ST_MSG "\n"
#define GGGG_ST_MSG "\n"
#define HHHH_ST_MSG "\n"
#define IIII_ST_MSG "\n"
#define JJJJ_ST_MSG "\n"
#define TRIGGER_UNKNOWN_ST_MSG "ERROR: Unknown state in Trigger\n"

// State machine states
enum trigger_st_st {
    INIT_ST,
    BBBB_ST,
    CCCC_ST,
    DDDD_ST,
    EEEE_ST,
    FFFF_ST,
    GGGG_ST,
    HHHH_ST,
    IIII_ST,
    JJJJ_ST
};
static enum trigger_st_st currentState;

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum trigger_st_st previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
        case INIT_ST:
            printf(INIT_ST_MSG);
            break;
        case BBBB_ST:
            printf(BBBB_ST_MSG);
            break;
        case CCCC_ST:
            printf(CCCC_ST_MSG);
            break;
        case DDDD_ST:
            printf(DDDD_ST_MSG);
            break;
        case EEEE_ST:
            printf(EEEE_ST_MSG);
            break;
        case FFFF_ST:
            printf(FFFF_ST_MSG);
            break;
        case GGGG_ST:
            printf(GGGG_ST_MSG);
            break;
        case HHHH_ST:
            printf(HHHH_ST_MSG);
            break;
        case IIII_ST:
            printf(IIII_ST_MSG);
            break;
        case JJJJ_ST:
            printf(JJJJ_ST_MSG);
            break;
        default:
            // Error message here
            printf(TRIGGER_UNKNOWN_ST_MSG);
            break;
     }
  }
}

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init() {
    currentState = INIT_ST;

};

// Standard tick function.
void trigger_tick() {
    
    // Optional debug messages
    if (DEBUG_TRIGGER) debugStatePrint();

     // Perform state update
    switch(currentState) {
        case INIT_ST:
            break;
        case BBBB_ST:
            break;
        case CCCC_ST:
            break;
        case DDDD_ST:
            break;
        case EEEE_ST:
            break;
        case FFFF_ST:
            break;
        case GGGG_ST:
            break;
        case HHHH_ST:
            break;
        case IIII_ST:
            break;
        case JJJJ_ST:
            break;
        default:
            // Error message here
            printf(TRIGGER_UNKNOWN_ST_MSG);
            break;
    }

    // Perform state actions
    switch(currentState) {
        case INIT_ST:
            break;
        case BBBB_ST:
            break;
        case CCCC_ST:
            break;
        case DDDD_ST:
            break;
        case EEEE_ST:
            break;
        case FFFF_ST:
            break;
        case GGGG_ST:
            break;
        case HHHH_ST:
            break;
        case IIII_ST:
            break;
        case JJJJ_ST:
            break;
        default:
            // Error message here
            printf(TRIGGER_UNKNOWN_ST_MSG);
            break;
    }
};

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() {

};

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() {

};

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {

};

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {

};

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest() {

};
