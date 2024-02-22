/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#ifndef LOCKOUTTIMER_H_
#define LOCKOUTTIMER_H_

#include <stdbool.h>

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init();

// Standard tick function.
void lockoutTimer_tick();

// Calling this starts the timer.
void lockoutTimer_start();

// Returns true if the timer is running.
bool lockoutTimer_running();

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest();

#endif /* LOCKOUTTIMER_H_ */
