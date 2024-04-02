/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "hitLedTimer.h"
#include "lockoutTimer.h"
#include "invincibilityTimer.h"
#include "transmitter.h"
#include "trigger.h"
#include "interrupts.h"
#include "buffer.h"
#include "sound.h"
#include "game.h"
#include "isr.h"
#include <stdio.h>

// The interrupt service routine (ISR) is implemented here.
// Add function calls for state machine tick functions and
// other interrupt related modules.

// Perform initialization for interrupt and timing related modules.
void isr_init() {

    // Call state machine initializations
    hitLedTimer_init();
    lockoutTimer_init();
    invincibilityTimer_tick();
    transmitter_init();
    trigger_init();
    buffer_init();
    sound_init();
};

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {

    // Call tick functions
    hitLedTimer_tick();
    lockoutTimer_tick();
    invincibilityTimer_tick();
    transmitter_tick();
    trigger_tick();
    sound_tick();
    // Get adc data
    buffer_pushover(interrupts_getAdcData());
};

