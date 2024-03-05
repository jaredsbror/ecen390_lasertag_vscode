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
#define ACTIVE_ST_MSG "active state\n"
#define INACTIVE_ST_MSG "inactive state\n"
#define LOCKOUT_TIMER_UNKNOWN_ST_MSG "ERROR: Unknown state in Lockout Timer\n"

// Global variables
static uint32_t tickCount;  // Tick counter
static bool active;     // Timer is active
static bool startTimer;     // Start the lockout timer

// State machine states
enum lockoutTimer_st_t {
    INIT_ST,
    INACTIVE_ST,
    ACTIVE_ST
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
        case ACTIVE_ST:
            printf(ACTIVE_ST_MSG);
            break;
        case INACTIVE_ST:
            printf(INACTIVE_ST_MSG);
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

    // Default integer values
    tickCount = 0;
    // Default boolean values
    active = false;
};

// Standard tick function.
void lockoutTimer_tick() {
    
    // Optional debug messages
    if (DEBUG_LOCKOUT_TIMER) debugStatePrint();

     // Perform state update
    switch(currentState) {

        case INIT_ST:
            // Immediately transition to inactive state
            currentState = INACTIVE_ST;
            // Default boolean values
            active = false;
            break;

        case INACTIVE_ST:
            // If timer is to be started, transition to active state
            if (startTimer) {
                currentState = ACTIVE_ST;
                // Reset tickCount to 0 and set active to true
                tickCount = 0;
                active = true;
            }
            break;

        case ACTIVE_ST:
            // If lockout timer is over, return to inactive state
            if (tickCount > LOCKOUT_TIMER_EXPIRE_VALUE) {
                currentState = INACTIVE_ST;
                //Reset tickCount to 0 and set active to false
                tickCount = 0;
                active = false;
            }
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
        case INACTIVE_ST:
            break;
        case ACTIVE_ST:
            // Increment tick counter
            tickCount++;
            break;
        default:
            // Error message here
            printf(LOCKOUT_TIMER_UNKNOWN_ST_MSG);
            break;
    }
};

// Calling this starts the timer.
void lockoutTimer_start() {
    startTimer = true;
};

// Returns true if the timer is running.
bool lockoutTimer_running() {
    return active;
};

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
    // Start an interval timer
    
    // Invoke lockoutTimer_start()

    // Wait while lockoutTimer_running() is true (another while-loop)

    // Once lockoutTimer_running() is false, stop the interval timer

    // Print out the time duration from the interval timer

};
