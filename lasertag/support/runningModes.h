/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#ifndef RUNNINGMODES_H_
#define RUNNINGMODES_H_

#include <stdint.h>

// Uncomment this code so that the code in the various modes will
// ignore your own frequency. You still must properly implement
// the ability to ignore frequencies in detector.c
#define IGNORE_OWN_FREQUENCY 1

#define MAX_HIT_COUNT 100000

#define MAX_BUFFER_SIZE 100 // Used for a generic message buffer.

#define DETECTOR_HIT_ARRAY_SIZE                                                \
  FILTER_FREQUENCY_COUNT // The array contains one location per user frequency.

#define HISTOGRAM_BAR_COUNT                                                    \
  FILTER_FREQUENCY_COUNT // As many histogram bars as user filter frequencies.

#define ISR_CUMULATIVE_TIMER INTERVAL_TIMER_TIMER_0 // Used by the ISR.
#define TOTAL_RUNTIME_TIMER                                                    \
  INTERVAL_TIMER_TIMER_1 // Used to compute total run-time.
#define MAIN_CUMULATIVE_TIMER                                                  \
  INTERVAL_TIMER_TIMER_2 // Used to compute cumulative run-time in main.

#define SYSTEM_TICKS_PER_HISTOGRAM_UPDATE                                      \
  30000 // Update the histogram about 3 times per second.

#define RUNNING_MODE_WARNING_TEXT_SIZE 2 // Upsize the text for visibility.
#define RUNNING_MODE_WARNING_TEXT_COLOR DISPLAY_RED // Red for more visibility.
#define RUNNING_MODE_NORMAL_TEXT_SIZE 1 // Normal size for reporting.
#define RUNNING_MODE_NORMAL_TEXT_COLOR DISPLAY_WHITE // White for reporting.
#define RUNNING_MODE_SCREEN_X_ORIGIN 0 // Origin for reporting text.
#define RUNNING_MODE_SCREEN_Y_ORIGIN 0 // Origin for reporting text.

// Detector should be invoked this often for good performance.
#define SUGGESTED_DETECTOR_INVOCATIONS_PER_SECOND 30000
// ADC queue should have no more than this number of unprocessed elements for
// good performance.
#define SUGGESTED_REMAINING_ELEMENT_COUNT 500

// Defined to make things more readable.
#define INTERRUPTS_CURRENTLY_ENABLED true
#define INTERRUPTS_CURRENTLY_DISABLE false

// Prints out various run-time statistics on the TFT display.
// Assumes the following:
// detected interrupts is retrieved with interrupts_isrInvocationCount(),
// interval_timer(0) is the cumulative run-time of the ISR,
// interval_timer(1) is the total run-time,
// interval_timer(2) is the time spent in main running the filters, updating the
// display, and so forth. No comments in the code, the print statements are
// self-explanatory.
void runningModes_printRunTimeStatistics(void);

// Group all of the inits together to reduce visual clutter.
void runningModes_initAll(void);

// Returns the current switch-setting
uint16_t runningModes_getFrequencySetting(void);

// This mode runs until BTN3 is pressed.
// When BTN3 is pressed, it exits and prints performance information to the TFT.
// Transmits continuously and displays the received power on the TFT.
// Transmit frequency is selected via the slide-switches.
void runningModes_continuous(void);

// This mode runs until BTN3 is pressed.
// When BTN3 is pressed, it exits and prints performance information to the TFT.
// Press BTN0 or the gun-trigger to shoot.
// Each shot is registered on the histogram on the TFT.
// Transmit frequency is selected via the slide-switches.
void runningModes_shooter(void);

// This mode simply dumps raw ADC values to the console.
// It can be used to determine if bipolar mode is working for the ADC.
// Will loop forever. Stop the program with an external reset or Ctl-C.
void runningModes_dumpRawAdcValues(void);

#endif /* RUNNINGMODES_H_ */
