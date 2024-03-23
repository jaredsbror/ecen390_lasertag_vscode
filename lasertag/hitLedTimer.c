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
#include "hitLedTimer.h"
#include "leds.h"
#include "mio.h"
#include "utils.h"
#include "buttons.h"
#include "detector.h"

#define DEBUG_HIT_LED_TIMER false  // If true, debug messages enabled

// The hitLedTimer is active for 1/2 second once it is started.
// While active, it turns on the LED connected to MIO pin 11
// and also LED LD0 on the ZYBO board.

#define HIT_LED_TIMER_EXPIRE_VALUE 50000 //correct 50000 // Defined in terms of 100 kHz ticks.
#define HIT_LED_TIMER_TEST_DELAY_VALUE 300 // Ms delay between tests
#define HIT_LED_TIMER_MILLISECOND_DELAY 1 // Slow down loop a little bit
#define HIT_LED_TIMER_OUTPUT_PIN 11      // JF-3
#define HIT_LED_TIMER_LED_JF3_HIGH 1
#define HIT_LED_TIMER_LED_JF3_LOW 0
#define HIT_LED_TIMER_LED_LD0_HIGH 0x1
#define HIT_LED_TIMER_LED_LD0_LOW 0x0

// All printed messages for states are provided here.
#define INIT_ST_MSG "init state\n"
#define NO_HIT_DETECTED_ST_MSG "no hit detected state\n"
#define HIT_DETECTED_ST_MSG "hit detected state\n"
#define HIT_LED_TIMER_UNKNOWN_ST_MSG "ERROR: Unknown state in Hit Led Timer\n"

// State machine states
enum hitLedTimer_st_t {
    INIT_ST,
    NO_HIT_DETECTED_ST,
    HIT_DETECTED_ST
};
static enum hitLedTimer_st_t currentState;

// Global variables
static uint32_t tickCounter;    // Tick counter
static bool hit;    // A hit was detected
static bool timer_enable;    // Timer is enabled

/////////////////////
// HELPER FUNCTIONS /
/////////////////////

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum hitLedTimer_st_t previousState;
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
        case NO_HIT_DETECTED_ST:
            printf(NO_HIT_DETECTED_ST_MSG);
            break;
        case HIT_DETECTED_ST:
            printf(HIT_DETECTED_ST_MSG);
            break;
        default:
            // Error message here
            printf(HIT_LED_TIMER_UNKNOWN_ST_MSG);
            break;
     }
  }
}

////////////////////////////
// STATE MACHINE FUNCTIONS /
////////////////////////////

// Need to init things.
void hitLedTimer_init() {

    currentState = INIT_ST;

    // Set hitLedTimer pin
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.

    // Intialize leds and buttons
    leds_init(true);
    buttons_init();

    // Default int values
    tickCounter = 0;
    
    // Default boolean values
    hit =  false;
    timer_enable = false;
};

// Standard tick function.
void hitLedTimer_tick() {
    
    // Optional debug messages
    if (DEBUG_HIT_LED_TIMER) debugStatePrint();

     // Perform state update
    switch(currentState) {

        case INIT_ST:
            // Default transition to inactive state
            currentState = NO_HIT_DETECTED_ST;
            hitLedTimer_turnLedOff();
            break;

        case NO_HIT_DETECTED_ST:
            // If a hit is detected and timer is enabled, move to hit_detected state
            // printf("Hit: %d, Enable: %d, Running: %d\n", hit, timer_enable, hitLedTimer_running());
            if (hit && timer_enable) {
                currentState = HIT_DETECTED_ST;
                // Set relevant variables
                tickCounter = 0;
                hitLedTimer_turnLedOn();
            }
            break;

        case HIT_DETECTED_ST:
            // If the timer time has expired, transition back to inactive state
            if (tickCounter >= HIT_LED_TIMER_EXPIRE_VALUE) {
                currentState = NO_HIT_DETECTED_ST;
                // Reset relevant variables
                hit = false; //
                tickCounter = 0;
                hitLedTimer_turnLedOff();
            }
            break;

        default:
            // Error message here
            printf(HIT_LED_TIMER_UNKNOWN_ST_MSG);
            break;
    }

    // Perform state actions
    switch(currentState) {
        case INIT_ST:
            break;
        case NO_HIT_DETECTED_ST:
            break;
        case HIT_DETECTED_ST:
            // Increment tick counter
            tickCounter++;
            break;
        default:
            // Error message here
            printf(HIT_LED_TIMER_UNKNOWN_ST_MSG);
            break;
    }
    
};

// Calling this starts the timer.
void hitLedTimer_start() {
    hit = true;
    // printf("Resuscitate LED's\n");
};

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return ((currentState == HIT_DETECTED_ST) || hit);
};

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, HIT_LED_TIMER_LED_JF3_HIGH); // Write a '1' to JF-3.

    // uint32_t current_led_value = (uint32_t) (getFudgeFactorIndex() << 1);  // Your current 32-bit LED output value

    // Define a mask with only bit0 set to 1
    // uint32_t mask = 1; // 0x1

    // Clear bit0 and set the new value (1)
    // uint32_t new_led_value = (current_led_value & ~mask) | 1;
    
    // leds_write(1);
};

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, HIT_LED_TIMER_LED_JF3_LOW); // Write a '0' to JF-3.

    // uint32_t current_led_value = (uint32_t) (getFudgeFactorIndex() << 1);  // Your current 32-bit LED output value

    // Define a mask with only bit0 set to 1
    // uint32_t mask = 1; // 0x1

    // Clear bit0 and set the new value (1)
    // uint32_t new_led_value = (current_led_value & ~mask) | 0;
    
    // leds_write(0);
};

// Disables the hitLedTimer.
void hitLedTimer_disable() {
    timer_enable = false;
};

// Enables the hitLedTimer.
void hitLedTimer_enable() {
    timer_enable = true;
};

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {
    // Initialize the machine
    hitLedTimer_init();
    hitLedTimer_enable(); // Sets enable to true
    hitLedTimer_start(); // Sets hit to true
    // printf("Testing\n");
    // Infinitely test the half second timer
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
        // While it is actively running
        while(hitLedTimer_running()){
            utils_msDelay(HIT_LED_TIMER_MILLISECOND_DELAY);
        };

        // Restart machine
        utils_msDelay(HIT_LED_TIMER_TEST_DELAY_VALUE);
        hitLedTimer_start();
   }
};
