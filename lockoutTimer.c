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
#include "lockoutTimer.h"

#define DEBUG_LOCKOUT_TIMER true  // If true, debug messages enabled

// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensures that only one hit is detected per 1/2-second interval.

#define LOCKOUT_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.

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
#define LOCKOUT_TIMER_UNKNOWN_ST_MSG "ERROR: Unknown state in Lockout Timer\n"

// State machine states
enum lockoutTimer_st_t {
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
static enum lockoutTimer_st_t currentState;

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum lockoutTimer_st_t previousState;
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
            printf(LOCKOUT_TIMER_UNKNOWN_ST_MSG);
            break;
     }
  }
}


// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
    currentState = INIT_ST;
};

// Standard tick function.
void lockoutTimer_tick() {
    
    // Optional debug messages
    if (DEBUG_LOCKOUT_TIMER) debugStatePrint();

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
            printf(LOCKOUT_TIMER_UNKNOWN_ST_MSG);
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
            printf(LOCKOUT_TIMER_UNKNOWN_ST_MSG);
            break;
    }
};

// Calling this starts the timer.
void lockoutTimer_start() {

};

// Returns true if the timer is running.
bool lockoutTimer_running() {

};

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {

};