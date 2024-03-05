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
#include "buttons.h"
#include "mio.h"
#include "utils.h"

#define DEBUG_TRIGGER true  // If true, debug messages enabled

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

// The trigger state machine must debounce both the press and the release of the gun trigger 
// (see Debouncing Switches).

// You read the current logic value of the trigger from pin JF-2 (MIO pin 10) which is connected 
// to the “Trigger” on the development board. You will ultimately connect the gun's trigger to the 
// “Trigger” pin. You can ignore the “Trigger” pin for now.

// Push-button BTN0 will also function as a trigger, i.e., when you press BTN0, everything will 
// operate just as if the gun were attached and the trigger was pulled (see Using BTN0 as a Trigger).
// Tie the trigger state machine to the transmitter. A trigger pull will invoke transmitter_run().

// The trigger state machine activates the transmitter state machine after the press of the trigger 
// has been debounced.

// The trigger state machine will not recognize another press of the trigger until the trigger has 
// been released and debounced.

// Each press/release of the trigger must activate the transmitter state machine only once.
// trigger_tick() must be invoked in isr_function() and nowhere else. Pass-off points will be 
// subtracted if trigger_tick() is invoked anywhere else.

// Debouncing values
#define TRIGGER_DEBOUNCE_PRESS_DELAY 5 // ticks, 50 ms
#define TRIGGER_DEBOUNCE_RELEASE_DELAY 5 // ticks, 50 ms

#define TRIGGER_TEST_TICK_PERIOD_IN_MS 10

// All printed messages for states are provided here.
#define INIT_ST_MSG "init state\n"
#define WAIT_FOR_PRESS_ST_MSG "wait for press state\n"
#define MAYBE_PRESSED_ST_MSG "maybe pressed state\n"
#define WAIT_FOR_RELEASE_ST_MSG "wait for release state\n"
#define MAYBE_RELEASED_ST_MSG "maybe released state\n"
#define TRIGGER_UNKNOWN_ST_MSG "ERROR: Unknown state in Trigger\n"

// State machine states
enum trigger_st_st {
    INIT_ST,                // 
    WAIT_FOR_PRESS_ST,      // 
    MAYBE_PRESSED_ST,       // 
    WAIT_FOR_RELEASE_ST,    // 
    MAYBE_RELEASED_ST,      // 
};
static enum trigger_st_st currentState;

// Global variables
static trigger_shotsRemaining_t shotsRemaining;
static uint32_t tickCount;
static bool buttonZeroPressed;
static bool enable;
static bool pressConfirmed;
static bool releaseConfirmed;

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
        case WAIT_FOR_PRESS_ST:
            printf(WAIT_FOR_PRESS_ST_MSG);
            break;
        case MAYBE_PRESSED_ST:
            printf(MAYBE_PRESSED_ST_MSG);
            break;
        case WAIT_FOR_RELEASE_ST:
            printf(WAIT_FOR_RELEASE_ST_MSG);
            break;
        case MAYBE_RELEASED_ST:
            printf(MAYBE_RELEASED_ST_MSG);
            break;
        default:
            // Error message here
            printf(TRIGGER_UNKNOWN_ST_MSG);
            break;
     }
  }
}

// // Trigger can be activated by either btn0 or the external gun that is attached to TRIGGER_GUN_TRIGGER_MIO_PIN
// // Gun input is ignored if the gun-input is high when the init() function is invoked.
// bool triggerPressed() {
// 	return ((!IGNORE_GUN_INPUT & (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) || 
//                 (buttons_read() & BUTTONS_BTN0_MASK));
// }

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init() {
    currentState = INIT_ST;

    // Init hardware
    buttons_init();
    // mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
    // // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
    // if (triggerPressed()) {
    //     ignoreGunInput = true;
    // }

    // Set variables
    shotsRemaining = 0;
    tickCount = 0;
    buttonZeroPressed = false;
    enable = false;
    pressConfirmed = false;
    releaseConfirmed = false;

    // printf("I AM INITED\n");
};

// Standard tick function.
void trigger_tick() {
    
    // printf("Tick tock tick\n");

    // Optional debug messages
    if (DEBUG_TRIGGER) debugStatePrint();

    // Read current buttons to find button 0
    buttonZeroPressed = (buttons_read() & BUTTONS_BTN0_MASK);

     // Perform state update
    switch(currentState) {
        case INIT_ST:
            // Default transition
            currentState = WAIT_FOR_PRESS_ST;
            break;

        case WAIT_FOR_PRESS_ST:
            // If a press is detected and machine is enabled, transition to MAYBE_PRESSED_ST
            if (enable && buttonZeroPressed) {
                currentState = MAYBE_PRESSED_ST;    // Transition
                // Set variables
                tickCount = 0;
                pressConfirmed = false;
                releaseConfirmed = false;
            }
            break;

        case MAYBE_PRESSED_ST:
            // If BTN0 is not pressed, return to WAIT_FOR_PRESS_ST
            if (!buttonZeroPressed) {
                currentState = WAIT_FOR_PRESS_ST;
            // Else if tickcount is greater than 50 ms, transition to WAIT_FOR_RELEASE_ST
            } else if (tickCount >= TRIGGER_DEBOUNCE_PRESS_DELAY) {
                currentState = WAIT_FOR_RELEASE_ST;
                tickCount = 0;
                pressConfirmed = true;
                releaseConfirmed = false;
            }
            break;

        case WAIT_FOR_RELEASE_ST:
            // If BTN0 is not pressed, continue to MAYBE_RELEASED_ST
            if (!buttonZeroPressed) {
                currentState = MAYBE_RELEASED_ST;   // Transition
                tickCount = 0;  // Set variables
            }
            break;

        case MAYBE_RELEASED_ST:
            // If BTN0 is pressed, return to WAIT_FOR_RELEASE_ST
            if (buttonZeroPressed) {
                currentState = WAIT_FOR_RELEASE_ST;
            // Else If tickcount is greater than 50 ms, transition to WAIT_FOR_PRESS_ST
            } else if (tickCount >= TRIGGER_DEBOUNCE_RELEASE_DELAY) {
                currentState = WAIT_FOR_PRESS_ST;
                tickCount = 0;
                pressConfirmed = false;
                releaseConfirmed = true;
            }
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
        case WAIT_FOR_PRESS_ST:
            break;
        case MAYBE_PRESSED_ST:
            // Increment tick counter
            tickCount++;
            break;
        case WAIT_FOR_RELEASE_ST:
            break;
        case MAYBE_RELEASED_ST:
            // Increment tick counter
            tickCount++;
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
    enable = true;
};

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() {
    enable = false;
};

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {
    return shotsRemaining;
};

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
    shotsRemaining = count;
};

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest() {
    // trigger_init();
    // trigger_enable();
    // while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
    //     // printf("Can you hear me???? I'm in the while loop in trigger.c runTest!\n");
    //     trigger_tick();
    //     utils_msDelay(TRIGGER_TEST_TICK_PERIOD_IN_MS);
    // }

    // Initialize the machine
    trigger_init();
    trigger_enable(); //sets enable to true
    // Infinitely test the trigger
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
        //
        
    
    };

};
