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

/*
  Game startup sound
  Got shot sound
  Lost a life sound
  Game over sound
  Return to base sound
  One second silence sound?


*/
// Frequencies
#define GAME_IGNORE_OWN_FREQUENCY 1
#define FREQUENCY_6 6
#define FREQUENCY_9 9
// Hits and lives
#define INITIAL_HIT_COUNT 0
#define INITIAL_LIFE_COUNT 3
#define HIT_COUNT_PER_LIFE 5
#define FIRST_LIFE 1
#define SECOND_LIFE 2
#define THIRD_LIFE 3
#define FIRST_LIFE_HIT_COUNT 5
#define SECOND_LIFE_HIT_COUNT 10
#define THIRD_LIFE_HIT_COUNT 15
// Delays
#define FINAL_DELAY 3
#define INVINCIBLE_DELAY_MS 5000

// Hits and lives
static uint32_t hitCount;
static uint32_t prevousLifeCount;
static uint32_t currentLifeCount;
// Status effects
static bool isGameOver;
static bool iAmInvincible;
// Ignored frequencies backup
static bool backupIgnoreFrequencies[FILTER_FREQUENCY_COUNT];

static void I_Am_Invincible() {
  
  // uint16_t previousHitCount = hitCount;

  // Delay for 5 seconds and flush adc buffer
  utils_msDelay(INVINCIBLE_DELAY_MS);
  detector_ignoreAllHits(true);
  detector(true);
  detector_ignoreAllHits(false);

  // Copy backup ignored frequencies to a detector array
  detector_setIgnoredFrequencies(backupIgnoreFrequencies);

  // Reset frequencies to ignore
  detector_clearHit();
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
  hitCount = INITIAL_HIT_COUNT;
  prevousLifeCount = INITIAL_LIFE_COUNT;
  currentLifeCount = INITIAL_LIFE_COUNT;
  isGameOver = false;
  iAmInvincible = false;

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

  // Copy ignored frequencies to a backup array
  for (uint32_t frequency = 0; frequency < FILTER_FREQUENCY_COUNT; frequency++) {
    backupIgnoreFrequencies[frequency] = ignoredFrequencies[frequency];
  }

  detector_setIgnoredFrequencies(ignoredFrequencies); // Set ignored frequencies

  printf("Ignored Frequencies(");
  // Iterate over the ignored frequencies
  for (uint32_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    printf("%d,", ignoredFrequencies[i]);
  }
  printf(")\n");

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
         hitCount < MAX_HIT_COUNT) { // Run until you detect BTN3 pressed.

    transmitter_setFrequencyNumber(runningModes_getFrequencySetting());    // Read the switches and switch
                                                // frequency as required.
    intervalTimer_start(MAIN_CUMULATIVE_TIMER); // Measure run-time when you are
                                                // doing something.
    // Run filters, compute power, run hit-detection.
    detector(INTERRUPTS_CURRENTLY_ENABLED); // Interrupts are currently enabled.
    
    // If a hit has been detected...
    if (detector_hitPreviouslyDetected()) {           // Hit detected
      hitCount++;                           // increment the hit count.
      detector_clearHit();                  // Clear the hit.
      detector_hitCount_t
          hitCounts[DETECTOR_HIT_ARRAY_SIZE]; // Store the hit-counts here.
      detector_getHitCounts(hitCounts);       // Get the current hit counts.
      histogram_plotUserHits(hitCounts);      // Plot the hit counts on the TFT.

      // Modify lifecount and determine if game is over
      // if (hitCount == FIRST_LIFE_HIT_COUNT)
      //   currentLifeCount = FIRST_LIFE;
      // else if (hitCount == SECOND_LIFE_HIT_COUNT)
      //   currentLifeCount = SECOND_LIFE;
      // else if (hitCount == THIRD_LIFE_HIT_COUNT)
      //   currentLifeCount = THIRD_LIFE;
      // else
      //   currentLifeCount = currentLifeCount;
      currentLifeCount = INITIAL_LIFE_COUNT - (hitCount / HIT_COUNT_PER_LIFE);

      // Check to see if a life has been lost...
      if (currentLifeCount < prevousLifeCount) {
        // Trigger I AM INVINCIBLE for 5 seconds
        sound_playSound(sound_loseLife_e);
        printf("You lost a life!\n");
        I_Am_Invincible();
        printf("I AM INVINCIBLE\n");
      } else {  // Player was simply hit
        sound_playSound(sound_hit_e);
        printf("You were shot!\n");
      }

      prevousLifeCount = currentLifeCount;  // Equalize the life counts
    }
    intervalTimer_stop(
        MAIN_CUMULATIVE_TIMER); // All done with actual processing.
  }

  sound_playSound(sound_gameOver_e);
  sound_waitForSoundToFinish();

  sound_playSound(sound_returnToBase_e);
  sound_waitForSoundToFinish();



  // Terminate the program completely
  interrupts_disableArmInts(); // Done with loop, disable the interrupts.
  hitLedTimer_turnLedOff();    // Save power :-)
  runningModes_printRunTimeStatistics(); // Print the run-time statistics.
  printf("Shooter mode terminated after detecting %d hits.\n", hitCount);
}
