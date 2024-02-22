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
#include "transmitter.h"

#define DEBUG_TRANSMITTER true  // If true, debug messages enabled

// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h
// #define TRANSMITTER_OUTPUT_PIN 13     // JF1 (pg. 25 of ZYBO reference manual).
// #define TRANSMITTER_PULSE_WIDTH 20000 // Based on a system tick-rate of 100 kHz.

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
#define TRANSMITTER_UNKNOWN_ST_MSG "ERROR: Unknown state in Transmitter\n"


// State machine states
enum transmitter_st_t {
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
static enum transmitter_st_t currentState;

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum transmitter_st_t previousState;
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
            printf(TRANSMITTER_UNKNOWN_ST_MSG);
            break;
     }
  }
}


// Standard init function.
void transmitter_init() {
    currentState = INIT_ST;
};

// Standard tick function.
void transmitter_tick() {
    
    // Optional debug messages
    if (DEBUG_TRANSMITTER) debugStatePrint();

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
            printf(TRANSMITTER_UNKNOWN_ST_MSG);
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
            printf(TRANSMITTER_UNKNOWN_ST_MSG);
            break;
    }
};

// Activate the transmitter.
void transmitter_run() {
    
};

// Returns true if the transmitter is still running.
bool transmitter_running() {
    
};

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    
};

// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() {
    
};

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, it
// transmits one burst and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until a 200 ms burst is complete. NOTE: while running continuously,
// the transmitter will only change frequencies in between 200 ms bursts.
void transmitter_setContinuousMode(bool continuousModeFlag) {
    
};

/******************************************************************************
***** Test Functions
******************************************************************************/

// Prints out the clock waveform to stdio. Terminates when BTN3 is pressed.
// Does not use interrupts, but calls the tick function in a loop.
void transmitter_runTest() {
    
};

// Tests the transmitter in non-continuous mode.
// The test runs until BTN3 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestNoncontinuous() {
    
};

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes in the slide switches.
// Test runs until BTN3 is pressed.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestContinuous() {
    
};
