/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

/*
The code in runningModes.c can be an example for implementing the game here.
*/

#include <stdio.h>

#include "sound.h"
#include "buffer.h"
#include "buttons.h"
#include "detector.h"
#include "display.h"
#include "filter.h"
#include "histogram.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "intervalTimer.h"
#include "isr.h"
#include "lockoutTimer.h"
#include "runningModes.h"
#include "switches.h"
#include "transmitter.h"
#include "trigger.h"
#include "utils.h"
#include "xparameters.h"
#include "invincibilityTimer.h"

/*
  Game startup sound
  Got shot sound
  Lost a life sound
  Game over sound
  Return to base sound
  One second silence sound?



*/
#define DEBUG_GAME false
// Frequencies
#define GAME_IGNORE_OWN_FREQUENCY
#define FREQUENCY_6 6
#define FREQUENCY_9 9
// Hits and lives
#define FIRST_LIFE_HIT_COUNT 5
#define SECOND_LIFE_HIT_COUNT 10
#define THIRD_LIFE_HIT_COUNT 15
#define MAX_NUMBER_HITS 15
// Delays
#define FINAL_DELAY 3
#define INVINCIBLE_DELAY_MS 5000

// Hits and lives
static uint32_t hitCount;
// Status effects
static bool isGameOver;

static void I_Am_Invincible() {

  // Optional global debug
  if (DEBUG_GAME) printf("I AM INVINCIBLE\n");
  invincibilityTimer_start();
  // Delay for 5 seconds and flush adc buffer
  utils_msDelay(INVINCIBLE_DELAY_MS);
  detector_ignoreAllHits(true);
  detector(true);
  detector_ignoreAllHits(false);
  // Reset frequencies to ignore 
  detector_clearHit();
  // Optional global debug
  if (DEBUG_GAME) printf("PLEASE DON'T SHOOT ME!\n");
};

// This game supports two teams, Team-A and Team-B.
// Each team operates on its own configurable frequency.
// Each player has a fixed set of lives and once they
// have expended all lives, operation ceases and they are told
// to return to base to await the ultimate end of the game.
// The gun is clip-based and each clip contains a fixed number of shots
// that takes a short time to reload a new clip.
// The clips are automatically loaded.
// Runs until BTN3 is pressed.
void game_twoTeamTag(void) {

  runningModes_initAll();
  sound_setVolume(sound_minimumVolume_e);

  // Initialize global variables
  hitCount = 0;

  sound_playSound(sound_gameStart_e);
  
  // Init the ignored-frequencies so no frequencies are ignored.
  bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
  // Iterate over the number of ignored frequencies array...
  for (uint32_t frequency = 0; frequency < FILTER_FREQUENCY_COUNT; frequency++) {
    // If a frequency is not frequency 6 or 9, it is ignored.
    ignoredFrequencies[frequency] = ((frequency != FREQUENCY_6) && (frequency != FREQUENCY_9) ? true : false);
  }
  #ifdef GAME_IGNORE_OWN_FREQUENCY
    printf("Ignoring own frequency.\n");
    ignoredFrequencies[runningModes_getFrequencySetting()] = true;
  #endif
  detector_setIgnoredFrequencies(ignoredFrequencies); // Set ignored frequencies

  // Optional global debug
  if (DEBUG_GAME) {
    printf("Ignored Frequencies(");
    // Iterate over the ignored frequencies
    for (uint32_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
      printf("%d,", ignoredFrequencies[i]);
    }
    printf(")\n");
  }

  // Configure timers and interrupts
  trigger_enable(); // Makes the state machine responsive to the trigger.
  interrupts_enableTimerGlobalInts(); // Allow timer interrupts.
  interrupts_startArmPrivateTimer();  // Start the private ARM timer running.
  intervalTimer_reset(ISR_CUMULATIVE_TIMER); // Used to measure ISR execution time.
  intervalTimer_reset(TOTAL_RUNTIME_TIMER); // Used to measure total program execution time.
  intervalTimer_reset(MAIN_CUMULATIVE_TIMER); // Used to measure main-loop execution time.
  intervalTimer_start(TOTAL_RUNTIME_TIMER);   // Start measuring total execution time.
  interrupts_enableArmInts(); // ARM will now see interrupts after this.
  lockoutTimer_start(); // Ignore erroneous hits at startup (when all power
                        // values are essentially 0).

  // Main game loop
  while ((!(buttons_read() & BUTTONS_BTN3_MASK)) &&
         hitCount < MAX_NUMBER_HITS) { // Run until you detect BTN3 pressed.

    transmitter_setFrequencyNumber(runningModes_getFrequencySetting());    // Read the switches and switch
                                                // frequency as required.
    intervalTimer_start(MAIN_CUMULATIVE_TIMER); // Measure run-time when you are
                                                // doing something.
    // Run filters, compute power, run hit-detection.
    if (!invincibilityTimer_running()) detector(INTERRUPTS_CURRENTLY_ENABLED); // Interrupts are currently enabled.
    
    // If a hit has been detected...
    if (detector_hitPreviouslyDetected() && !invincibilityTimer_running()) {           // Hit detected
      hitCount++;                           // increment the hit count.
      detector_hitCount_t
          hitCounts[DETECTOR_HIT_ARRAY_SIZE]; // Store the hit-counts here.
      detector_getHitCounts(hitCounts);       // Get the current hit counts.
      histogram_plotUserHits(hitCounts);      // Plot the hit counts on the TFT.
      detector_clearHit();                  // Clear the hit.

      // Check to see if a life has been lost...
      if ((hitCount == FIRST_LIFE_HIT_COUNT) || (hitCount == SECOND_LIFE_HIT_COUNT)) {
        // Optional global debug
        if (DEBUG_GAME) printf("HitCount: %d\n", hitCount);
        // Trigger I AM INVINCIBLE for 5 seconds
        sound_playSound(sound_loseLife_e);
        // Optional global debug
        if (DEBUG_GAME) printf("Lost a life\n");
        I_Am_Invincible();
      } else if (hitCount == THIRD_LIFE_HIT_COUNT) {
        sound_playSound(sound_gameOver_e);
        break;
      } else {  // Player was simply hit
        // Optional global debug
        if (DEBUG_GAME) printf("HitCount: %d\n", hitCount);
        sound_playSound(sound_hit_e);
        // Optional global debug
        if (DEBUG_GAME) printf("Got hit\n");
      }
    }
    intervalTimer_stop(
        MAIN_CUMULATIVE_TIMER); // All done with actual processing.
  }

  sound_waitForSoundToFinish();

  sound_playSound(sound_returnToBase_e);
  sound_waitForSoundToFinish();

  // Terminate the program completely
  interrupts_disableArmInts(); // Done with loop, disable the interrupts.
  hitLedTimer_turnLedOff();    // Save power :-)
  runningModes_printRunTimeStatistics(); // Print the run-time statistics.
  printf("Shooter mode terminated after detecting %d hits.\n", hitCount);
}
