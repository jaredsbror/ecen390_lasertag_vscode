/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/
 
// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by the detector.
// The function of the buffer is similar to a buffer or FIFO.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "buffer.h"

#define BUFFER_SIZE 32768
 
typedef struct {
    // Points to the next open slot.
    uint32_t indexIn;
    // Points to the next element to be removed.
    uint32_t indexOut;
    // Number of elements in the buffer.
    uint32_t elementCount;
    // Values are stored here.
    buffer_data_t data[BUFFER_SIZE]; 
} buffer_t;

volatile static buffer_t buffer;

// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by the detector.
// The function of the buffer is similar to a buffer or FIFO.

///////////////////////
/// HELPER FUNCTIONS //
///////////////////////

// Return true if the buffer is empty
static bool buffer_empty() {
    return buffer.elementCount == 0;
}

// Return true if the buffer is full
static bool buffer_full() {
    return buffer.elementCount == BUFFER_SIZE;
}

// Clear the buffer
void buffer_clear() {
    // Reset all elements of the buffer to zero
    for (uint32_t index = 0; index < BUFFER_SIZE; index++) {
        buffer.data[index] = 0;
    }
}

/////////////////////
/// MAIN FUNCTIONS //
/////////////////////

// Initialize the buffer to empty.
void buffer_init(void) {
    // Reset buffer struct variables to zero or empty
    buffer.indexIn = 0;
    buffer.indexOut = 0;
    buffer.elementCount = 0;

    // Reset all elements to zero
    buffer_clear();
};

// Remove a value from the buffer. Return zero if empty.
buffer_data_t buffer_pop(void) {
    // Check for an empty buffer
    if (buffer_empty()) {
        // IF the buffer is empty, print an error
        // message and DO NOT change the buffer.
        printf("ERROR in buffer_pop(): Buffer is empty. Unable to pop.\n");
        return 0;
    }

    // If the buffer is not empty, remove and return the oldest element in the
    // buffer. Pop from buffer using MAGIC!
    buffer_data_t extractedData = buffer.data[buffer.indexOut];

    // Decrement element number and increment indexOut
    buffer.elementCount--;
    buffer.indexOut++;
    buffer.indexOut = (buffer.indexOut) % BUFFER_SIZE;

    // Return a buffer_data_t value
    return extractedData;
};

// Add a value to the buffer. Overwrite the oldest value if full.
void buffer_pushover(buffer_data_t value) {
    
    // If the buffer is full, call buffer_pop before adding a value
    if (buffer_full()) buffer_pop();
    
    // If the buffer is not full, push the new element to the buffer
    // Push to buffer using MAGIC!
    buffer.data[buffer.indexIn] = value;

    // Increment element number and indexIn
    buffer.elementCount++;
    buffer.indexIn++;
    buffer.indexIn = (buffer.indexIn) % BUFFER_SIZE;
};

// Return the number of elements in the buffer.
uint32_t buffer_elements(void) {
    return buffer.elementCount;
};

// Return the capacity of the buffer in elements.
uint32_t buffer_size(void) {
    return BUFFER_SIZE;
};
