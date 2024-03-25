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
#include "filter.h"
#include "mio.h"
#include "buttons.h"
#include "switches.h"
#include "utils.h"

#define DEBUG_TRANSMITTER false         // If true, debug messages enabled (slower)
#define DEBUG_TRANSMITTER_TICKS false   // If true, tick count messages enabled (very slow) 

// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h
#define TRANSMITTER_OUTPUT_PIN 13     // JF1 (pg. 25 of ZYBO reference manual).
#define TRANSMITTER_PULSE_WIDTH 20000 // Based on a system tick-rate of 100 kHz.
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0

// All printed messages for states are provided here.
#define INIT_ST_MSG "init state\n"
#define INACTIVE_ST_MSG "default state\n"
#define ON_ST_MSG "on state\n"
#define OFF_ST_MSG "off state\n"
#define TRANSMITTER_UNKNOWN_ST_MSG "ERROR: Unknown state in Transmitter\n"

#define DIVIDE_BY_TWO 2

// Global variables
volatile static uint16_t frequency;     // Player number (index from o to 9 instead of 1 to 10)
volatile static uint16_t newFrequency;  // Updated player number to be applied next cycle
volatile static uint16_t frequencyTicks;    // Number of ticks representing player frequency
volatile static uint16_t newFrequencyTicks; // Updated number of ticks for next cycle
volatile static bool triggerPulled;     // Gun trigger pulled
volatile static bool continuousMode;    // State machine should transmit continuously
static bool on;     // Whether the state machine is active or not
 
// State machine states
enum transmitter_st_t {
    INIT_ST,
    INACTIVE_ST,
    ON_ST,
    OFF_ST
};
static enum transmitter_st_t currentState;

/////////////////////
// HELPER FUNCTIONS /
/////////////////////

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
            printf("\n");
            printf(INIT_ST_MSG);
            break;
        case INACTIVE_ST:
            printf("\n");
            printf(INACTIVE_ST_MSG);
            break;
        case ON_ST:
            printf("\n");
            printf("Player %d with half ticks %d --> ", frequency + 1, frequencyTicks);
            printf(ON_ST_MSG);
            break;
        case OFF_ST:
            printf("\n");
            printf("Player %d with half ticks %d --> ", frequency + 1, frequencyTicks);
            printf(OFF_ST_MSG);
            break;
        default:
            // Error message here
            printf(TRANSMITTER_UNKNOWN_ST_MSG);
            break;
     }
  }
}

// Set transmitter pin to 1
static void transmitter_set_jf1_to_one() {
  mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-1.
}

// Set transmitter pin to 0
static void transmitter_set_jf1_to_zero() {
  mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '0' to JF-1.
}

////////////////////////////
// STATE MACHINE FUNCTIONS /
////////////////////////////

// Standard init function.
void transmitter_init() {
    // Set transmitter pint
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.
    
    // Integer default values
    frequency = 0;
    newFrequency = 0;
    frequencyTicks = 0;
    newFrequencyTicks = 0;

    // Boolean default values
    triggerPulled = false;
    continuousMode = false;
    on = false;

    // Transition
    currentState = INIT_ST;
    // printf("initialized\n");
};

// Standard tick function.
void transmitter_tick() {
    // Transmitter tick count
    static uint32_t transmitterTick;

    
    // Optional debug messages
    if (DEBUG_TRANSMITTER) debugStatePrint();

     // Perform state update
    switch(currentState) {
        case INIT_ST:
            currentState = INACTIVE_ST;  // Transition to default state
            break;

        case INACTIVE_ST:
            // If trigger is pulled or continuous mode is active, turn on pulsing for 200 ms
            if(triggerPulled || continuousMode){
                currentState = ON_ST;
                // Reset variables for next 200ms pulse
                on = true;
                triggerPulled = false;
                transmitterTick = 1;
                // Update frequency and frequencyTicks values for next pulse
                frequency = newFrequency;
                frequencyTicks = newFrequencyTicks;
                // Optional debug: printf("frequency = %d\n", frequency);
                //Set JF1 pin to ON when transistion to ON_ST
                transmitter_set_jf1_to_one();
            } else{
                // Set on to false
                on = false;
            }
            break;

        case ON_ST:
            // Optional debug print
            if (DEBUG_TRANSMITTER) printf("1");
            // Check whether or not to terminate the pulse output
            if (transmitterTick > TRANSMITTER_PULSE_WIDTH) {
                currentState = INACTIVE_ST;
                transmitterTick = 0;
                transmitter_set_jf1_to_zero();
            // Transition to off state depending on frequency
            } else if((transmitterTick % frequencyTicks) == 0) {
                currentState = OFF_ST;
                // Optional debug print
                if (DEBUG_TRANSMITTER) printf("\n");
                //Set JF1 pin to OFF when transistion to OFF_ST
                transmitter_set_jf1_to_zero();
            }
            break;

        case OFF_ST:

            // Optional debug print
            if (DEBUG_TRANSMITTER) printf("0");
            // Check whether or not to terminate the pulse output
            if (transmitterTick > TRANSMITTER_PULSE_WIDTH) {
                currentState = INACTIVE_ST;
                // Optional debug print
                if (DEBUG_TRANSMITTER) printf("\n");
                transmitterTick = 0;
            // Transition to on state depending on frequency
            } else if((transmitterTick % frequencyTicks) == 0) {
                currentState = ON_ST;
                //Set JF1 pin to ON when transistion to ON_ST
                transmitter_set_jf1_to_one();
            }
    
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
        case INACTIVE_ST:
            break;
        case ON_ST:
            // Optional debug statement
            if (DEBUG_TRANSMITTER_TICKS) printf("%d -> ",transmitterTick);
            // Increment tick count
            transmitterTick++;
            // Optional debug statement
            if (DEBUG_TRANSMITTER_TICKS) printf("%d\n",transmitterTick);
            break;
        case OFF_ST:
            // Increment tick count
            transmitterTick++;
            break;
        default:
            // Error message here
            printf(TRANSMITTER_UNKNOWN_ST_MSG);
            break;
    }
};

// Activate the transmitter.
void transmitter_run() {
    triggerPulled = true;
};

// Returns true if the transmitter is still running.
bool transmitter_running() {
    return on;
};

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    // Set frequency ONLY IF state machine is currently inactive
    if (currentState == INACTIVE_ST) {
        // Divide by 2 to get half cycles
        // Optional debug // printf("newFrequency = %d\n", frequencyNumber);
        newFrequencyTicks = filter_getFrequencyTick(frequencyNumber) / DIVIDE_BY_TWO;
        newFrequency = frequencyNumber;
    }
};
  
// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() {
    return frequency;
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
    continuousMode = continuousModeFlag;
};

/******************************************************************************
***** Test Functions
******************************************************************************/

#define BOUNCE_DELAY 5
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
#define TRANSMITTER_NONCONTINUOUS_TEST_MS_DELAY 400
// Prints out the clock waveform to stdio. Terminates when BTN1 is pressed.
// Prints out one line of 1s and 0s that represent one period of the clock signal, in terms of ticks.
void transmitter_runTest() {
    printf("starting transmitter_runTest()\n");
    transmitter_init();                                // init the transmitter.
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {        // Run continuously until BTN3 is pressed.
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
        transmitter_run();                                    // Start the transmitter.
        while (transmitter_running()) {                       // Keep ticking until it is done.
            transmitter_tick();                                 // tick.
            utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
        }
        printf("completed one test period.\n");
    }
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    printf("exiting transmitter_runTest()\n");
}

// Tests the transmitter in non-continuous mode.
// The test runs until BTN3 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestNoncontinuous() {
    // Set variables
    continuousMode = false;
    printf("starting transmitter_runTestNoncontinuous()\n");
    transmitter_init();                                // init the transmitter.
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {        // Run continuously until BTN3 is pressed.
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
        transmitter_run();                                    // Start the transmitter.
        while (transmitter_running()) {                       // Keep ticking until it is done.
            transmitter_tick();                                 // tick.
            utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
        }
        printf("completed one test period.\n");

        // Delay rerun of test depending on continuousMode
        utils_msDelay(TRANSMITTER_NONCONTINUOUS_TEST_MS_DELAY);
    }
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    printf("exiting transmitter_runTestNoncontinuous()\n");
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
    // Set variables
    continuousMode = true;
    printf("starting transmitter_runTestNoncontinuous()\n");
    transmitter_init();                                // init the transmitter.
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {        // Run continuously until BTN3 is pressed.
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
        transmitter_run();                                    // Start the transmitter.
        while (transmitter_running()) {                       // Keep ticking until it is done.
            transmitter_tick();                                 // tick.
            utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
        }
        printf("completed one test period.\n");
    }
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    printf("exiting transmitter_runTestNoncontinuous()\n");
};
