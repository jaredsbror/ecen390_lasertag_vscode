/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#ifndef TRIGGER_H_
#define TRIGGER_H_

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10     // JF2 (pg. 25 of ZYBO reference manual).

// Debouncing values
#define TRIGGER_DEBOUNCE_PRESS_DELAY 50 // ticks, 50 ms
#define TRIGGER_DEBOUNCE_RELEASE_DELAY 50 // ticks, 50 ms
#define TRIGGER_DEBOUNCE_MILLISECOND_DELAY 1    // Slow down the loop

#define SHOT_COUNT_MAX 10
#define TRIGGER_RELOAD_AUTOMATIC_DELAY_TICKS 300000
#define TRIGGER_CHARGED_SHOT_DELAY_TICKS 300000

#define TEAM_A_DEFAULT_SHOOT_FREQUENCY 6
#define TEAM_A_CHARGED_SHOOT_FREQUENCY 7
// Team B
#define TEAM_B_DEFAULT_SHOOT_FREQUENCY 9
#define TEAM_B_CHARGED_SHOOT_FREQUENCY 8
 
#include <stdint.h>

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

typedef uint16_t trigger_shotsRemaining_t;

// Set isTeamA
void trigger_setIsTeamA(bool teamA);

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init();

// Standard tick function.
void trigger_tick();

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable();

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable();

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount();

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count);

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest();

#endif /* TRIGGER_H_ */
