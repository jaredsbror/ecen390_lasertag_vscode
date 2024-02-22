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

// The hitLedTimer is active for 1/2 second once it is started.
// While active, it turns on the LED connected to MIO pin 11
// and also LED LD0 on the ZYBO board.

// Need to init things.
void hitLedTimer_init() {

};

// Standard tick function.
void hitLedTimer_tick() {

};

// Calling this starts the timer.
void hitLedTimer_start() {

};

// Returns true if the timer is currently running.
bool hitLedTimer_running() {

};

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {

};

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {

};

// Disables the hitLedTimer.
void hitLedTimer_disable() {

};

// Enables the hitLedTimer.
void hitLedTimer_enable() {

};

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {

};
