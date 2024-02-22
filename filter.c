#include "filter.h"
#include "queue.h"
#include <stdint.h>
#include <stdio.h>

#define X_QUEUE_SIZE 81
#define Y_QUEUE_SIZE 11
#define Z_QUEUE_SIZE 10
#define Z_QUEUE_FILTER_SIZE 10
#define OUTPUT_QUEUE_SIZE 10
#define OUTPUT_QUEUE_DATA_SIZE 2000
#define POWER_ARRAY_SIZE 10
#define FREQUENCY_COUNT 10
#define PLAYER_COUNT 10
#define IIR_A_COEFFICIENTS_COUNT 10
#define IIR_B_COEFFICIENTS_COUNT 11
#define FIR_COEFFICIENTS_COUNT 81

#define POWER_200_SIZE 200

static queue_t xQueue;
static queue_t yQueue;
static queue_t zQueues[Z_QUEUE_SIZE];
static queue_t outputQueues[OUTPUT_QUEUE_SIZE];

static double powerArray[POWER_ARRAY_SIZE];

const static double fir_coeffs[FIR_COEFFICIENTS_COUNT] = {
6.3536354866876830e-04, 
6.0819331227618758e-04, 
5.1686902749145777e-04, 
3.4863476412320759e-04, 
9.0970145666490485e-05, 
-2.6013348499733977e-04, 
-6.9113201237783293e-04, 
-1.1636939136330157e-03, 
-1.6118557625036021e-03, 
-1.9455697539817289e-03, 
-2.0615266547871075e-03, 
-1.8606534819201439e-03, 
-1.2700955552915063e-03, 
-2.6610384649516049e-04, 
1.1066405062375218e-03, 
2.7238875057424570e-03, 
4.3827874873925013e-03, 
5.8173566216966838e-03, 
6.7286366879234167e-03, 
6.8272100805459425e-03, 
5.8832490071413189e-03, 
3.7772815455755565e-03, 
5.4373527845984399e-04, 
-3.6006507470023436e-03, 
-8.2501610536106201e-03, 
-1.2829327634690009e-02, 
-1.6638812392493308e-02, 
-1.8924891946279925e-02, 
-1.8965758131617549e-02, 
-1.6164784711183367e-02, 
-1.0139023184532614e-02, 
-7.9083828812502024e-04, 
1.1648073345203356e-02, 
2.6608113314638429e-02, 
4.3212309435229891e-02, 
6.0344682425207186e-02, 
7.6750218817119298e-02, 
9.1155872838272686e-02, 
1.0239892938244033e-01, 
1.0954768753648120e-01, 
1.1200000000000000e-01, 
1.0954768753648120e-01, 
1.0239892938244033e-01, 
9.1155872838272686e-02, 
7.6750218817119298e-02, 
6.0344682425207186e-02, 
4.3212309435229891e-02, 
2.6608113314638429e-02, 
1.1648073345203356e-02, 
-7.9083828812502024e-04, 
-1.0139023184532614e-02, 
-1.6164784711183367e-02, 
-1.8965758131617549e-02, 
-1.8924891946279925e-02, 
-1.6638812392493308e-02, 
-1.2829327634690009e-02, 
-8.2501610536106201e-03, 
-3.6006507470023436e-03, 
5.4373527845984399e-04, 
3.7772815455755565e-03, 
5.8832490071413189e-03, 
6.8272100805459425e-03, 
6.7286366879234167e-03, 
5.8173566216966838e-03, 
4.3827874873925013e-03, 
2.7238875057424570e-03, 
1.1066405062375218e-03, 
-2.6610384649516049e-04, 
-1.2700955552915063e-03, 
-1.8606534819201439e-03, 
-2.0615266547871075e-03, 
-1.9455697539817289e-03, 
-1.6118557625036021e-03, 
-1.1636939136330157e-03, 
-6.9113201237783293e-04, 
-2.6013348499733977e-04, 
9.0970145666490485e-05, 
3.4863476412320759e-04, 
5.1686902749145777e-04, 
6.0819331227618758e-04, 
6.3536354866876830e-04};

const static double
    iir_a_coeffs[FILTER_FREQUENCY_COUNT][IIR_A_COEFFICIENTS_COUNT] = {
        {-5.9637727070164033e+00, 1.9125339333078255e+01,
         -4.0341474540744194e+01, 6.1537466875368864e+01,
         -7.0019717951472245e+01, 6.0298814235238922e+01,
         -3.8733792862566332e+01, 1.7993533279581079e+01,
         -5.4979061224867714e+00, 9.0332828533799669e-01},
        {-4.6377947119071452e+00, 1.3502215749461570e+01,
         -2.6155952405269751e+01, 3.8589668330738334e+01,
         -4.3038990303252618e+01, 3.7812927599537105e+01,
         -2.5113598088113765e+01, 1.2703182701888075e+01,
         -4.2755083391143422e+00, 9.0332828533800047e-01},
        {-3.0591317915750920e+00, 8.6417489609637457e+00,
         -1.4278790253808829e+01, 2.1302268283304279e+01,
         -2.2193853972079200e+01, 2.0873499791105413e+01,
         -1.3709764520609372e+01, 8.1303553577931549e+00,
         -2.8201643879900469e+00, 9.0332828533799914e-01},
        {-1.4071749185996754e+00, 5.6904141470697507e+00,
         -5.7374718273676271e+00, 1.1958028362868889e+01,
         -8.5435280598354506e+00, 1.1717345583835939e+01,
         -5.5088290876998514e+00, 5.3536787286077505e+00,
         -1.2972519209655555e+00, 9.0332828533799692e-01},
        {8.2010906117760374e-01, 5.1673756579268622e+00, 3.2580350909220956e+00,
         1.0392903763919200e+01, 4.8101776408669146e+00, 1.0183724507092519e+01,
         3.1282000712126798e+00, 4.8615933365572053e+00, 7.5604535083145064e-01,
         9.0332828533800169e-01},
        {2.7080869856154490e+00, 7.8319071217995537e+00, 1.2201607990980708e+01,
         1.8651500443681556e+01, 1.8758157568004464e+01, 1.8276088095998929e+01,
         1.1715361303018827e+01, 7.3684394621253011e+00, 2.4965418284511713e+00,
         9.0332828533799669e-01},
        {4.9479835250075865e+00, 1.4691607003177580e+01, 2.9082414772101004e+01,
         4.3179839108869231e+01, 4.8440791644688744e+01, 4.2310703962394200e+01,
         2.7923434247706322e+01, 1.3822186510470948e+01, 4.5614664160654126e+00,
         9.0332828533799459e-01},
        {6.1701893352279829e+00, 2.0127225876810328e+01, 4.2974193398071662e+01,
         6.5958045321253437e+01, 7.5230437667866596e+01, 6.4630411355739867e+01,
         4.1261591079244141e+01, 1.8936128791950544e+01, 5.6881982915180354e+00,
         9.0332828533799925e-01},
        {7.4092912870072407e+00, 2.6857944460290131e+01, 6.1578787811202240e+01,
         9.8258255839887298e+01, 1.1359460153696294e+02, 9.6280452143026054e+01,
         5.9124742025776371e+01, 2.5268527576524200e+01, 6.8305064480743063e+00,
         9.0332828533799947e-01},
        {8.5743055776347745e+00, 3.4306584753117932e+01, 8.4035290411037238e+01,
         1.3928510844056856e+02, 1.6305115418161682e+02, 1.3648147221895846e+02,
         8.0686288623300157e+01, 3.2276361903872299e+01, 7.9045143816245220e+00,
         9.0332828533800280e-01}};

const static double
    iir_b_coeffs[FILTER_FREQUENCY_COUNT][IIR_B_COEFFICIENTS_COUNT] = {
        {9.0928661148193611e-10, 0.0000000000000000e+00,
         -4.5464330574096802e-09, 0.0000000000000000e+00,
         9.0928661148193603e-09, 0.0000000000000000e+00,
         -9.0928661148193603e-09, 0.0000000000000000e+00,
         4.5464330574096802e-09, 0.0000000000000000e+00,
         -9.0928661148193611e-10},
        {9.0928661148195566e-10, 0.0000000000000000e+00,
         -4.5464330574097786e-09, 0.0000000000000000e+00,
         9.0928661148195572e-09, 0.0000000000000000e+00,
         -9.0928661148195572e-09, 0.0000000000000000e+00,
         4.5464330574097786e-09, 0.0000000000000000e+00,
         -9.0928661148195566e-10},
        {9.0928661148191140e-10, 0.0000000000000000e+00,
         -4.5464330574095569e-09, 0.0000000000000000e+00,
         9.0928661148191138e-09, 0.0000000000000000e+00,
         -9.0928661148191138e-09, 0.0000000000000000e+00,
         4.5464330574095569e-09, 0.0000000000000000e+00,
         -9.0928661148191140e-10},
        {9.0928661148212833e-10, 0.0000000000000000e+00,
         -4.5464330574106413e-09, 0.0000000000000000e+00,
         9.0928661148212827e-09, 0.0000000000000000e+00,
         -9.0928661148212827e-09, 0.0000000000000000e+00,
         4.5464330574106413e-09, 0.0000000000000000e+00,
         -9.0928661148212833e-10},
        {9.0928661148195421e-10, 0.0000000000000000e+00,
         -4.5464330574097711e-09, 0.0000000000000000e+00,
         9.0928661148195423e-09, 0.0000000000000000e+00,
         -9.0928661148195423e-09, 0.0000000000000000e+00,
         4.5464330574097711e-09, 0.0000000000000000e+00,
         -9.0928661148195421e-10},
        {9.0928661148191626e-10, 0.0000000000000000e+00,
         -4.5464330574095809e-09, 0.0000000000000000e+00,
         9.0928661148191618e-09, 0.0000000000000000e+00,
         -9.0928661148191618e-09, 0.0000000000000000e+00,
         4.5464330574095809e-09, 0.0000000000000000e+00,
         -9.0928661148191626e-10},
        {9.0928661148210362e-10, 0.0000000000000000e+00,
         -4.5464330574105181e-09, 0.0000000000000000e+00,
         9.0928661148210362e-09, 0.0000000000000000e+00,
         -9.0928661148210362e-09, 0.0000000000000000e+00,
         4.5464330574105181e-09, 0.0000000000000000e+00,
         -9.0928661148210362e-10},
        {9.0928661148193291e-10, 0.0000000000000000e+00,
         -4.5464330574096644e-09, 0.0000000000000000e+00,
         9.0928661148193289e-09, 0.0000000000000000e+00,
         -9.0928661148193289e-09, 0.0000000000000000e+00,
         4.5464330574096644e-09, 0.0000000000000000e+00,
         -9.0928661148193291e-10},
        {9.0928661148185877e-10, 0.0000000000000000e+00,
         -4.5464330574092939e-09, 0.0000000000000000e+00,
         9.0928661148185877e-09, 0.0000000000000000e+00,
         -9.0928661148185877e-09, 0.0000000000000000e+00,
         4.5464330574092939e-09, 0.0000000000000000e+00,
         -9.0928661148185877e-10},
        {9.0928661148189176e-10, 0.0000000000000000e+00,
         -4.5464330574094585e-09, 0.0000000000000000e+00,
         9.0928661148189169e-09, 0.0000000000000000e+00,
         -9.0928661148189169e-09, 0.0000000000000000e+00,
         4.5464330574094585e-09, 0.0000000000000000e+00,
         -9.0928661148189176e-10}};

// Initialize xQueue
void initXQueue() {
  // Init xQueue
  queue_init(&xQueue, X_QUEUE_SIZE, "xQueue");
  // Fill queue with 0's
  for (int i = 0; i < X_QUEUE_SIZE; i++) {
    queue_overwritePush(&xQueue, 0);
  }
}

// Initialize yQueue
void initYQueue() {
  // Init yQueue
  queue_init(&yQueue, Y_QUEUE_SIZE, "yQueue");
  // Fill queue with 0's
  for (int i = 0; i < Y_QUEUE_SIZE; i++) {
    queue_overwritePush(&yQueue, 0);
  }
}

// Initialize zQueue
void initZQueue() {
  // Iterate through zQueue
  for (int i = 0; i < Z_QUEUE_SIZE; i++) {

    // Custom naming
    char name[20];
    sprintf(name, "zQueue%d", i);
    // Init zQueue
    queue_init(&zQueues[i], Z_QUEUE_SIZE, name);

    // Fill queue with 0's
    for (int j = 0; j < Z_QUEUE_FILTER_SIZE; j++) {
      queue_overwritePush(&zQueues[i], 0);
    }
  }
}

// Initialize outputQueues
void initOutputQueue() {
  // Iterate through outputQueues
  for (int i = 0; i < OUTPUT_QUEUE_SIZE; i++) {
    // Custom name
    char name[20];
    sprintf(name, "outputQueue%d", i);
    // Init outputQueues
    queue_init(&outputQueues[i], OUTPUT_QUEUE_DATA_SIZE, name);
    // Fill queue with 0's
    for (int j = 0; j < OUTPUT_QUEUE_DATA_SIZE; j++) {
      queue_overwritePush(&outputQueues[i], 0);
    }
  }
}

// Initialize power queue
void initPowerArray() {
  // Fill Array with 0's
  for (int i = 0; i < POWER_ARRAY_SIZE; i++) {
    powerArray[i] = 0;
  }
}

/******************************************************************************
***** Main Filter Functions
******************************************************************************/

// Must call this prior to using any filter functions.
void filter_init() {
  initXQueue();
  initYQueue();
  initZQueue();
  initOutputQueue();
  initPowerArray();
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x) { queue_overwritePush(&xQueue, x); }

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter() {
  double sum = 0;
  // Iterate through xQueue
  for (int i = 0; i < FIR_COEFFICIENTS_COUNT; i++) {
    sum += queue_readElementAt(&xQueue, FIR_COEFFICIENTS_COUNT - 1 - i) *
           fir_coeffs[i];
  }
  // Push sum to yQueue
  queue_overwritePush(&yQueue, sum);
  return sum;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber) {

  // Declare sum variables
  double sumY = 0;
  double sumZ = 0;
  double sumYminusZ = 0;

  // Iterate through y-queue
  for (int k = 0; k < IIR_B_COEFFICIENTS_COUNT; k++) {
    sumY += queue_readElementAt(&yQueue, IIR_B_COEFFICIENTS_COUNT - 1 - k) *
            iir_b_coeffs[filterNumber][k];
  }

  // Iterate through z-queue instance
  for (int k = 0; k < IIR_A_COEFFICIENTS_COUNT; k++) {
    sumZ += queue_readElementAt(&zQueues[filterNumber],
                                IIR_A_COEFFICIENTS_COUNT - 1 - k) *
            iir_a_coeffs[filterNumber][k];
  }

  // Push new values to zQueue and outputQueues
  sumYminusZ = sumY - sumZ;
  queue_overwritePush(&zQueues[filterNumber], sumYminusZ);
  queue_overwritePush(&outputQueues[filterNumber], sumYminusZ);
  return sumYminusZ;
};

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute
// power values the first time. After that, you can incrementally compute
// power values by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run,
// call this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
// (newest-value * newest-value). Note that this function will probably need
// an array to keep track of these values for each of the 10 output queues.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch,
                           bool debugPrint) {
  double newPower;
  static double oldestValue[FREQUENCY_COUNT] = {0};
  // If force == true, then recompute power by using all values in the
  // outputQueue.
  if (forceComputeFromScratch) {
    newPower = 0;
    // Recompute power by using all values in the outputQueue
    for (int i = 0; i < queue_elementCount(&outputQueues[filterNumber]); i++) {
      double value = queue_readElementAt(&outputQueues[filterNumber], i);
      newPower += value * value;
    }

  } else {
    // Keeping track of the power computed in a previous run, call this
    // prev-power.
    double prevPower = powerArray[filterNumber];
    // Keeping track of the oldest outputQueue value used in a previous run,
    // call this oldest-value.

    // 3. Get the newest value from the output queue, call this newest-value.
    double newestValue = queue_readElementAt(
        &outputQueues[filterNumber],
        queue_elementCount(&outputQueues[filterNumber]) - 1);
    // 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
    // (newest-value * newest-value). Note that this function will probably need
    // an array to keep track of these values for each of the 10 output queues.
    newPower = prevPower -
               (oldestValue[filterNumber] * oldestValue[filterNumber]) +
               (newestValue * newestValue);
  }

  oldestValue[filterNumber] =
      queue_readElementAt(&outputQueues[filterNumber], 0);
  powerArray[filterNumber] = newPower; // Add to power array
  return newPower;
};

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber) {
  return powerArray[filterNumber];
};

// Sets a current power value for a specific filter number.
// Useful in testing the detector.
void filter_setCurrentPowerValue(uint16_t filterNumber, double value) {
  powerArray[filterNumber] = value;
};

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes
// to the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]) {
  // Copy the power values from the power queue to the powerValues array
  for (int i = 0; i < POWER_ARRAY_SIZE; i++) {
    powerValues[i] = powerArray[i];
  }
};

// Using the previously-computed power values that are currently stored in
// currentPowerValue[] array, copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in
// currentPowerValue[]. The pointer argument indexOfMaxValue is used to return
// the index of the maximum value. If the maximum power is zero, make sure to
// not divide by zero and that *indexOfMaxValue is initialized to a sane value
// (like zero).
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue) {
  // Using the previously-computed power values that are currently stored in
  // currentPowerValue[] array, copy these values into the normalizedArray[]
  // argument
  filter_getCurrentPowerValues(normalizedArray);

  // Calculate max power of the array in order to normalize it
  double maxPower = 0;
  uint16_t maxIndex = 0;
  // Iterate through the power array and calculate maximum power value/index
  for (uint16_t i = 0; i < POWER_ARRAY_SIZE; i++) {
    // If current power value is greater than max power value, update max
    if (normalizedArray[i] > maxPower) {
      maxPower = normalizedArray[i];
      maxIndex = i;
    }
  }

  // Return (1) the index of the max power and (2) the normalized array
  if (maxPower == 0) {
    *indexOfMaxValue = 0;
  } else {
    *indexOfMaxValue = maxIndex;
    // Because maxPower is not 0, we can normalize the array
    for (int i = 0; i < POWER_ARRAY_SIZE; i++) {
      normalizedArray[i] /= maxPower;
    }
  }
};

/******************************************************************************
***** Verification-Assisting Functions
***** External test functions access the internal data structures of filter.c
***** via these functions. They are not used by the main filter functions.
******************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray() { return fir_coeffs; };

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount() { return FIR_COEFFICIENTS_COUNT; };

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {
  return iir_a_coeffs[filterNumber];
};

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() { return IIR_A_COEFFICIENTS_COUNT; };

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {
  return iir_b_coeffs[filterNumber];
};

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() { return IIR_B_COEFFICIENTS_COUNT; };

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() { return Y_QUEUE_SIZE; };

// Returns the decimation value.
uint16_t filter_getDecimationValue() { return Z_QUEUE_FILTER_SIZE; };

// Returns the address of xQueue.
queue_t *filter_getXQueue() { return &xQueue; };

// Returns the address of yQueue.
queue_t *filter_getYQueue() { return &yQueue; };

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) {
  return &zQueues[filterNumber];
};

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {
  return &outputQueues[filterNumber];
};

// Returns the frequency tick count given a frequency number
const uint16_t filter_getFrequencyTick(uint16_t frequencyNumber) {
  return filter_frequencyTickTable[frequencyNumber];
};