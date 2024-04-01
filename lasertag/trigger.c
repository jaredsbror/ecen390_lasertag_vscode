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
#include "transmitter.h"
#include "sound.h"
#include "invincibilityTimer.h"

// Uncomment for debug prints
#define DEBUG_SINGLE_LETTER_PRINTOUTS false  // Single letter debug messages
#define DEBUG_TRIGGER false  // If true, debug messages enabled
#define DEBUG_RELOAD false

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

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10     // JF2 (pg. 25 of ZYBO reference manual).

// Debouncing values
#define TRIGGER_DEBOUNCE_PRESS_DELAY 50 // ticks, 50 ms
#define TRIGGER_DEBOUNCE_RELEASE_DELAY 50 // ticks, 50 ms
#define TRIGGER_DEBOUNCE_MILLISECOND_DELAY 1    // Slow down the loop

// All printed messages for states are provided here.
#define INIT_ST_MSG "init state\n"
#define WAIT_FOR_PRESS_ST_MSG "wait for press state\n"
#define MAYBE_PRESSED_ST_MSG "maybe pressed state\n"
#define WAIT_FOR_RELEASE_ST_MSG "wait for release state\n"
#define MAYBE_RELEASED_ST_MSG "maybe released state\n"
#define TRIGGER_UNKNOWN_ST_MSG "ERROR: Unknown state in Trigger\n"

#define SHOT_COUNT_MAX 10
#define TRIGGER_RELOAD_AUTOMATIC_DELAY_TICKS 300000
#define TRIGGER_RELOAD_MANUAL_DELAY_TICKS 300000

// State machine states
enum trigger_st_st {
    INIT_ST,                // Initialize the machine
    WAIT_FOR_PRESS_ST,      // Waiting to be pressed
    MAYBE_PRESSED_ST,       // Waiting a certain time period to confirm button press
    WAIT_FOR_RELEASE_ST,    // Waiting to be released
    MAYBE_RELEASED_ST,      // Waiting a certain time period to confirm button release
};
static enum trigger_st_st currentState;

// Global variables
// Numbers
volatile static trigger_shotsRemaining_t shotsRemaining;    // Remaining shots
volatile static uint32_t mainTickCount; // Main tick count (used for half second trigger delay until next shot)
volatile static uint32_t reloadTickCount; // Reload tick count (used to measure three second reload delay)
// Booleans
volatile static bool enable;    // Enable trigger SM
volatile static bool pressConfirmed;    // Confirm a press
volatile static bool releaseConfirmed;  // Confirm a release
volatile static bool ignoreGunInput;    // Ignore gun trigger input



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

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {
    return shotsRemaining;
};

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
    shotsRemaining = count;
};


// Trigger can be activated by either btn0 or the external gun that is attached to TRIGGER_GUN_TRIGGER_MIO_PIN
// Gun input is ignored if the gun-input is high when the init() function is invoked.
bool triggerPressed() {
	return ((!ignoreGunInput && (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == 1)) || 
                (buttons_read() & BUTTONS_BTN0_MASK));
}

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init() {
    currentState = INIT_ST;

    // Init hardware
    buttons_init();

    // Load the gun
    trigger_setRemainingShotCount(SHOT_COUNT_MAX);

    // Determine whether to ignore gun trigger input or just use BTN0
    ignoreGunInput = false;
    mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
    // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
    if (triggerPressed()) {
        ignoreGunInput = true;
    }

    // Set variables to default
    mainTickCount = 0;
    reloadTickCount = 0;
    enable = false;
    pressConfirmed = false;
    releaseConfirmed = false;

    
};

static void trigger_outOfAmmo(void){
    sound_playSound(sound_gunClick_e);
    // Optional global debug
    if (DEBUG_RELOAD) printf("EMPTY\n");
}

static void trigger_fire(void){
    shotsRemaining--;
    // If sound is not busy, play the sound
    // if (!sound_isBusy()) 
    sound_playSound(sound_gunFire_e);
    // Optional global debug
    if (DEBUG_RELOAD) printf("FIRE\n");
}

// Reload gun with max bullets in clip
static void trigger_reload(bool isAutomatic) {
    trigger_setRemainingShotCount(SHOT_COUNT_MAX);
    sound_playSound(sound_gunReload_e);
    // Optional global debug
    if (DEBUG_RELOAD) {
        printf(isAutomatic ? "AUTOMATIC " : "MANUAL ");
        printf("RELOAD\n");
    }
}

// Standard tick function.
void trigger_tick(void) {
    static uint32_t reload_ticks_automatic;
    static uint32_t reload_ticks_manual;

    // Optional debug messages
    if (DEBUG_TRIGGER) debugStatePrint();

     // Perform state update
    switch(currentState) {
        case INIT_ST:
            // Default transition
            reload_ticks_automatic = 0;
            reload_ticks_manual = 0;
            currentState = WAIT_FOR_PRESS_ST;
            break;

        case WAIT_FOR_PRESS_ST:
            // If a press is detected and machine is enabled, transition to MAYBE_PRESSED_ST
            if (enable && triggerPressed()) {
                currentState = MAYBE_PRESSED_ST;    // Transition
                // Set variables
                mainTickCount = 0;
                pressConfirmed = false;
                releaseConfirmed = false;
            }
            else if ((reload_ticks_automatic >= TRIGGER_RELOAD_AUTOMATIC_DELAY_TICKS) && (shotsRemaining == 0)) {
                // Reset reload tick counts
                reload_ticks_automatic = 0;
                reload_ticks_manual = 0;
                trigger_reload(true);
            }
            break;

        case MAYBE_PRESSED_ST:
            // If BTN0 is not pressed, return to WAIT_FOR_PRESS_ST
            if (!triggerPressed()) {
                currentState = WAIT_FOR_PRESS_ST;
            // Else if mainTickCount is greater than 50 ms, transition to WAIT_FOR_RELEASE_ST
            } else if (mainTickCount >= TRIGGER_DEBOUNCE_PRESS_DELAY) {

                currentState = WAIT_FOR_RELEASE_ST; // Transition to next state
                
                // Change button booleans
                pressConfirmed = true;
                releaseConfirmed = false;

                // Only fire if !invincibilityTimer_running()
                if (!invincibilityTimer_running()) {
                    // Run the transmitter
                    transmitter_run();
                    // Sound depends on shots remaining
                    if (shotsRemaining != 0) {
                        trigger_fire();
                    } else {
                        trigger_outOfAmmo();
                    }
                }
                
                // Reset tick counts
                mainTickCount = 0;
                reload_ticks_automatic = 0;
                reload_ticks_manual = 0;
                // Print out char
                if (DEBUG_SINGLE_LETTER_PRINTOUTS) printf("D\n");
            }
            break;

        case WAIT_FOR_RELEASE_ST:
            // If BTN0 is not pressed, continue to MAYBE_RELEASED_ST
            if (!triggerPressed()) {
                currentState = MAYBE_RELEASED_ST;   // Transition
                mainTickCount = 0;  // Set variables
            }
            else if (reload_ticks_manual >= TRIGGER_RELOAD_MANUAL_DELAY_TICKS && !invincibilityTimer_running()) {    
                // Reset reload tick counts
                reload_ticks_automatic = 0;
                reload_ticks_manual = 0;
                trigger_reload(false);
            }
            break;

        case MAYBE_RELEASED_ST:
            // If BTN0 is pressed, return to WAIT_FOR_RELEASE_ST
            if (triggerPressed()) {
                currentState = WAIT_FOR_RELEASE_ST;
            // Else If mainTickCount is greater than 50 ms, transition to WAIT_FOR_PRESS_ST
            } else if (mainTickCount >= TRIGGER_DEBOUNCE_RELEASE_DELAY) {
                currentState = WAIT_FOR_PRESS_ST;
                mainTickCount = 0;
                pressConfirmed = false;
                releaseConfirmed = true;
                // Print out char
                if (DEBUG_SINGLE_LETTER_PRINTOUTS) printf("U\n");
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
            // Only increment manual reload ticks if !invincibilityTimer_running()
            if (!invincibilityTimer_running()) reload_ticks_manual++;
            // Only increment automatic reload ticks if you're out of ammo
            if(shotsRemaining == 0) reload_ticks_automatic++;
    
            break;
        case MAYBE_PRESSED_ST:
            // Increment tick counter
            mainTickCount++;
            break;
        case WAIT_FOR_RELEASE_ST:
            // Only increment manual reload ticks if !invincibilityTimer_running()
            if (!invincibilityTimer_running()) reload_ticks_manual++;
        case MAYBE_RELEASED_ST:
            // Increment tick counter
            mainTickCount++;
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

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest() {

    // Initialize the machine
    trigger_init();
    trigger_enable(); //sets enable to true
    // Infinitely test the trigger
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
        // Slow down the machine
        utils_msDelay(TRIGGER_DEBOUNCE_MILLISECOND_DELAY);
    };

};
