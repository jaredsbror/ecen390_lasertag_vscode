

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
#include "hitLedTimer.h"
#include "utils.h"


#define DEBUG_DETECTOR false


// Used to normalize the adc inputs to values between -1 and 1
#define HALF_OF_MAX_ADC_VALUE 2047.5
// 
#define MEDIAN_POWER_VALUE_INDEX 4
#define FUDGE_FACTOR 25
#define FUDGE_FACTOR_DEFAULT_INDEX 4

#define QUEUE_1 {10, 20, 3000, 40, 50, 60, 70, 80, 2000, 15}
#define QUEUE_2  {10, 20, 3000, 40, 500, 60, 70, 80, 10, 15}
#define QUEUE_3 {10, 20, 30, 40, 20, 60, 70, 80, 100, 3000}


// 
typedef uint16_t detector_hitCount_t;


// Global variables
static uint64_t invocationCount;    // Number of times detector is called

static bool ignoreAllHits;  // If true, ignore all hits
static uint16_t frequencyNumberOfLastHit;   // Frequency of last hit

static const uint32_t FUDGE_FACTORS[FILTER_FREQUENCY_COUNT];    // Possible fudge factors
static uint32_t fudge_factor_index; // Fudge factor array index
static uint32_t fudge_factor; // this is our fudge factor, but this is so we can modify the fudge factor later and iterate through.

static bool ignoredPlayerFrequencies[FILTER_FREQUENCY_COUNT];   // Ignored player frequencies
static double powerValues[FILTER_FREQUENCY_COUNT];  // Unsorted power values
static double powerValues_sorted[FILTER_FREQUENCY_COUNT];   // Sorted power values
static uint16_t playerFrequencies_sorted[FILTER_FREQUENCY_COUNT];   // Player frequencies
static detector_hitCount_t detectorHitArray[FILTER_FREQUENCY_COUNT];    // Player hits

static bool detector_hitDetectedFlag;   // Hit detected


// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void) {
   // Reset booleans
   ignoreAllHits = false;
   detector_hitDetectedFlag = false;


   // Reset numbers
   fudge_factor_index = FUDGE_FACTOR_DEFAULT_INDEX;
   invocationCount = 0;
   frequencyNumberOfLastHit = 0;
   fudge_factor = FUDGE_FACTOR;


   // Set all frequencies to not be ignored
   // Also reset other relevant arrays that
   // have the same size
   for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
       ignoredPlayerFrequencies[i] = true;
       powerValues[i] = 0.0;
       powerValues_sorted[i] = 0.0;
       playerFrequencies_sorted[i] = 0;
       detectorHitArray[i] = 0;
   }

};


// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]) {
    
   // Set ignored player frequencies
   for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
       ignoredPlayerFrequencies[i] = freqArray[i];
   }
};

// Swap two doubles in an array
static void swapDoubles(double* a, double* b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

// Swap two ints
static void swapInts(uint16_t* a, uint16_t* b) {
    uint16_t temp = *a;
    *a = *b;
    *b = temp;
}

// Selection sort of an array 
static void selectionSort(double arr[], uint16_t indexes[], uint16_t size) {
    uint16_t i, j, max_idx;
    // Iterate through size - 1
    for (i = 0; i < size - 1; i++) {
        max_idx = i;
        // Iterate through size
        for (j = i + 1; j < size; j++) {
            // Compare power values (currently in ascending order)
            if (arr[j] < arr[max_idx]) { // Change to > for descending order
                max_idx = j;
            }
        }
        swapDoubles(&arr[max_idx], &arr[i]);
        swapInts(&indexes[max_idx], &indexes[i]);
    }
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void) {
   // Set global detection variable to false
   detector_hitDetectedFlag = false;
};


// Detect a hit
bool detector_hitCurrentlyDetected() {
    // Optional debug statement
    if (DEBUG_DETECTOR) printf("STARTING: detector_hitCurrentlyDetected\n");

    // Get current power values
    filter_getCurrentPowerValues(powerValues);
    
    // Sort the power values
    // 1) Copy the values to powerValues_sorted and reset playerFrequencies_sorted
    for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
        powerValues_sorted[i] = powerValues[i];
        playerFrequencies_sorted[i] = i;
    }
    // 2) Use selection sort algorithm
    selectionSort(powerValues_sorted, playerFrequencies_sorted, FILTER_FREQUENCY_COUNT);

    // Calculate median power value and baseline power
    double powerValue_median = powerValues_sorted[MEDIAN_POWER_VALUE_INDEX];
    double base_line = powerValue_median * fudge_factor;

    // Reset hitDetected;
    detector_clearHit();
    // Iterate through the sorted power values array...
    for (int32_t i = FILTER_FREQUENCY_COUNT - 1; i >= 0; i--) {
        // If the associated frequency is not ignored...
        if (!ignoredPlayerFrequencies[playerFrequencies_sorted[i]]) {
            // If the power value is greater than the base_line...
            if (powerValues_sorted[i] > base_line) {
                // Register the hit where needed
                detector_hitDetectedFlag = true;
                frequencyNumberOfLastHit = playerFrequencies_sorted[i];
                detectorHitArray[playerFrequencies_sorted[i]] += 1;

                // Optional debug statement
                if (DEBUG_DETECTOR) {
                    printf("detectorHitArray {");
                    // Print out power values for debug
                    for (int32_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
                        printf("%d", detectorHitArray[i]);
                        printf((i < FILTER_FREQUENCY_COUNT - 1 ? "," : ""));
                    }
                    printf("}\n");
                }

                return detector_hitDetectedFlag;
            }
        }
    }

    // Optional debug statement
    if (DEBUG_DETECTOR) printf("TERMINATING: detector_hitCurrentlyDetected()\n");
    return detector_hitDetectedFlag;
}

// Returns true if a hit was detected.
bool detector_hitPreviouslyDetected(void) {
   return detector_hitDetectedFlag;
};


// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void) {
   return frequencyNumberOfLastHit;
};

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) {
   ignoreAllHits = flagValue;
};


// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
   // Iterate through detectorHitArray...
   for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
       // Copy the values over
       hitArray[i] = detectorHitArray[i];
   }
};


// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factorIdx) {
   fudge_factor_index = factorIdx;
};

// Get the fudge facter externally
uint32_t getFudgeFactorIndex() {
    return fudge_factor_index;
}


// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void) {
   return invocationCount;
};

// interrupts are running. If interruptsCurrentlyEnabled = false you can pop
// values from the ADC buffer without disabling interrupts. If
// interruptsCurrentlyEnabled = true, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC buffer.
// 3. re-enable interrupts.
// Ignore hits on frequencies specified with detector_setIgnoredFrequencies().
// Assumption: draining the ADC buffer occurs faster than it can fill.
void detector(bool interruptsCurrentlyEnabled){

   // Increment the invocation count
   invocationCount++;

   // Query the ADC buffer to determine how many elements it contains.
   uint32_t elementCount = buffer_elements();
   static uint32_t invoke_filter = 0;


   // Repeat the following steps elementCount times
   for (int32_t i = 0; i < elementCount; i++){
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

           // If the filter has been invoked more than 10 times...
           if (invoke_filter == FILTER_FREQUENCY_COUNT){
               invoke_filter = 0;
               filter_firFilter();
               // Iterate through filters for each frequency
               for (int32_t filterNumber = 0; filterNumber < FILTER_FREQUENCY_COUNT; filterNumber++) {
                    filter_iirFilter(filterNumber); // Run each of the IIR filters.
                    // Compute the power for each of the filters, at lowest computational cost.
                    // 1st false means do not compute from scratch.
                    // 2nd false means no debug prints.
                    filter_computePower(filterNumber, false, false);
                }

                // Optional debug statement
                if (DEBUG_DETECTOR) {
                    printf("PowerValues {");
                    // Print out power values for debug
                    for (int32_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
                        printf("%d", powerValues[i]);
                        printf((i < FILTER_FREQUENCY_COUNT - 1 ? "," : ""));
                    }
                    printf("}\n");
                }

               // if the lockoutTimer is not running, run the hit-detection algorithm...
               if (lockoutTimer_running() == false) {

                   // If you detect a hit and the frequency with maximum power is
                   // not an ignored frequency...
                   if (detector_hitCurrentlyDetected()) {
                        
                        lockoutTimer_start();   // Start lockoutTimer
                        hitLedTimer_enable();   // Start hitLedTimer (line 1)
                        hitLedTimer_start();    // Start hitLedTimer (line 2)
                       
                   }
               }
           }
       }
   }
}


/******************************************************
******************** Test Routines ********************
******************************************************/


// Students implement this as part of Milestone 3, Task 3.
// Create two sets of power values and call your hit detection algorithm
// on each set. With the same fudge factor, your hit detect algorithm
// should detect a hit on the first set and not detect a hit on the second.
void detector_runTest(void) {
    printf("STARTING: Detector_runTest()\n");
    detector_init();
    
    //  Set ignored frequencies
    bool ignored_frequencies[FILTER_FREQUENCY_COUNT] = {false};
    detector_setIgnoredFrequencies(ignored_frequencies);


    uint32_t power_Values[FILTER_FREQUENCY_COUNT] = QUEUE_1;
    // Populate current power values
    for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++){
        filter_setCurrentPowerValue(i, power_Values[i]);
    }
    // Print out result
    if (detector_hitCurrentlyDetected()) printf("Hit detected at Frequency %d\n", frequencyNumberOfLastHit);
    else printf("Hit not detected\n");
  

    uint32_t power_Values2[FILTER_FREQUENCY_COUNT] = QUEUE_2;
    // Populate current power values
    for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++){
        filter_setCurrentPowerValue(i, power_Values2[i]);
    }
    // Print out result
    if (detector_hitCurrentlyDetected()) printf("Hit detected at Frequency %d\n", frequencyNumberOfLastHit);
    else printf("Hit not detected\n");


    uint32_t power_Values3[FILTER_FREQUENCY_COUNT] = QUEUE_3;
    // Populate current power values
    for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++){
        filter_setCurrentPowerValue(i, power_Values3[i]);
    }
    // Print out result
    if (detector_hitCurrentlyDetected()) printf("Hit detected at Frequency %d\n", frequencyNumberOfLastHit);
    else printf("Hit not detected\n");


    printf("TERMINATING: Detector_runTest()\n");
};