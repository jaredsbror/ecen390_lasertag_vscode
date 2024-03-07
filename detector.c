
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
#include "interrupts.h"
#include "buffer.h"
#include "filter.h"
#include "lockoutTimer.h"

// Used to normalize the adc inputs to values between -1 and 1
#define HALF_OF_MAX_ADC_VALUE 2047.5
// 
#define MEDIAN_POWER_VALUE_INDEX 4
#define FUDGE_FACTOR 1.4

//
typedef uint16_t detector_hitCount_t;

// Global variables
volatile static bool hitDetected;
volatile static bool ignoreAllHits;
// Unchanging
const volatile uint32_t FUDGE_FACTORS[FILTER_FREQUENCY_COUNT];
// Changing
volatile static bool ignoredPlayerFrequencies[FILTER_FREQUENCY_COUNT];
volatile static double powerValues[FILTER_FREQUENCY_COUNT];
volatile static double powerValues_sorted[FILTER_FREQUENCY_COUNT];
volatile static uint16_t playerFrequencies_sorted[FILTER_FREQUENCY_COUNT];

volatile static u_int16_t fudge_factor_index = 4;   //??? Put in init later



// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void) {
    hitDetected = false;
    ignoreAllHits = false;
};

// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]) {

};

// Runs the entire detector: decimating FIR-filter, IIR-filters,
// power-computation, hit-detection. If interruptsCurrentlyEnabled = true,
// interrupts are running. If interruptsCurrentlyEnabled = false you can pop
// values from the ADC buffer without disabling interrupts. If
// interruptsCurrentlyEnabled = true, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC buffer.
// 3. re-enable interrupts.
// Ignore hits on frequencies specified with detector_setIgnoredFrequencies().
// Assumption: draining the ADC buffer occurs faster than it can fill.


// Now, repeat the following steps elementCount times.
    // If interrupts are enabled (check to see if the interruptsEnabled argument == true), 
        // briefly disable interrupts by invoking interrupts_disableArmInts().
    // Pop a value from the ADC buffer (use buffer_pop() for this). 
    // Place this value in a variable called rawAdcValue.
    // If the interruptsEnabled argument was true, re-enable interrupts by invoking interrupts_enableArmInts().
    // Scale the integer value contained in rawAdcValue to a double that is between -1.0 and 1.0. 
    // Store this value into a variable named scaledAdcValue.

    // Invoke filter_addNewInput(scaledAdcValue). This provides a new input to the FIR filter.

    // If filter_addNewInput() has been called 10 times since the last invocation of the FIR and IIR filters, 
    // run the FIR filter, IIR filter and power computation for all 10 channels. 
    // Remember to only invoke these filters and power computations after filter_addNewInput() has been called 10 times (decimation). 
    
    // If you have just run the filters and computed power, also do the following:
    // if the lockoutTimer is not running, run the hit-detection algorithm. 
        // If you detect a hit and the frequency with maximum power is not an ignored frequency, do the following:
            // start the lockoutTimer.
            // start the hitLedTimer.
            // increment detector_hitArray at the index of the frequency of the IIR-filter output where you detected the hit. 
            // Note that detector_hitArray is a 10-element integer array that simply holds the current number of hits, 
            // for each frequency, that have occurred to this point.
            // set detector_hitDetectedFlag to true.
void detector(bool interruptsCurrentlyEnabled){

    // Query the ADC buffer to determine how many elements it contains.
    uint32_t elementCount = buffer_elements();
    static uint16_t invoke_filter = 0;

    // Repeat the following steps elementCount times
    for (int i = 0; i < buffer_elements(); i++){\
        // If interrupts are currently enabled...
        if (interruptsCurrentlyEnabled) {
            // Temporarily disable interrupts to pop a value from the buffer
            interrupts_disableArmInts();
            buffer_data_t rawAdcValue = buffer_pop();
            interrupts_enableArmInts();

            // Scale the value to between -1 and 1 and send the value through the filters
            double scaledAdcValue = (double)(rawAdcValue) / HALF_OF_MAX_ADC_VALUE - 1.0;
            filter_addNewInput(scaledAdcValue);
            invoke_filter++;

            // If the 
            if (invoke_filter > 10){
                invoke_filter = 0;
                filter_firFilter();
                filter_iirFilter(0);
                filter_computePower(0, false, false);
                // if the lockoutTimer is not running, run the hit-detection algorithm. 
                if (lockoutTimer_running() == false){

                    // If you detect a hit and the frequency with maximum power is not an ignored frequency
                    if (detector_hitDetected() && ){
                        lockoutTimer_start();   // Start lockoutTimer
                        hitLedTimer_enable();   // Start hitLedTimer (line 1)
                        hitLedTimer_start();    // Start hitLedTimer (line 2)
                    }
                }
            }
        }
    }
}

// Returns true if a hit was detected.
bool detector_hitDetected(void) {
    // 
    filter_getCurrentPowerValues(powerValues);
    
    int most = 0;
    int most_place = 0;
    //
    double powerValues_sorted[FILTER_FREQUENCY_COUNT];
    // 
    for(uint32_t i = 0; i < FILTER_FREQUENCY_COUNT; i++){
        // 
        for(uint32_t j = i; j < FILTER_FREQUENCY_COUNT; j++){
            // 
            if(powerValues[j] > most){
                most = powerValues[j];
                most_place = j;
            }
        }

        //
        powerValues_sorted[i] = most;
        powerValues[most_place] = 0;
        most = 0;
    }
    // 
    double powerValue_median = powerValues_sorted[fudge_factor_index];
    double base_line = powerValue_median * FUDGE_FACTOR;
    
    // If the first sorted power value (the greatest) is greater than the base_line
    hitDetected = (powerValues_sorted[0] > base_line ? true : false);
    for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++){
        if(powerValues[i] == powerValues_sorted[0]){
            
        }
    }

    return hitDetected;
};

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void) {

};

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void) {
    // Set global detection variable to false
    hitDetected = false;
};

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) [
    ignoreAllHits = flagValue;
];

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {

};

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factorIdx) {
    fudge_factor_index = factorIdx;
};

// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void) {

};

/******************************************************
******************** Test Routines ********************
******************************************************/

// Students implement this as part of Milestone 3, Task 3.
// Create two sets of power values and call your hit detection algorithm
// on each set. With the same fudge factor, your hit detect algorithm
// should detect a hit on the first set and not detect a hit on the second.
void detector_runTest(void) {

};
