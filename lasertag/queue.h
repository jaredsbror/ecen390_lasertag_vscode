//
// Created by joshua on 1/11/24.
//

#ifndef LASER_TAG_QUEUE_H
#define LASER_TAG_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

// Limit the size of the statically-allocated queue name.
#define QUEUE_MAX_NAME_SIZE 50

// Return this when queue_pop(), queue_readElementAt() needs to return something
// during an error condition.
#define QUEUE_RETURN_ERROR_VALUE ((queue_data_t)0)

// Big enough to address everything in the queue.
typedef uint32_t queue_index_t;

// Just make everything double for this project.
typedef double queue_data_t;

// Not sure if we need something different from the index type.
typedef uint32_t queue_size_t;

// The queue struct with elementCount to speed up computations to determine
// element count. Queue will use the empty location and pointer arithmetic to
// determine full and empty.
typedef struct {
    // Always points to the next open slot.
    queue_index_t indexIn;

    // Always points to the next element to be removed
    // from the queue (or "oldest" element).
    queue_index_t indexOut;

    // Keep track of the number of elements currently in queue.
    queue_size_t elementCount;

    // This is the size of the data array. Actual queue
    // capacity is one less.
    queue_size_t size;

    // Points to a dynamically-allocated array.
    queue_data_t *data;

    // True if queue_pop() is called on an empty queue. Reset
    // to false after queue_push() is called.
    bool underflowFlag;

    // True if queue_push() is called on a full queue. Reset to
    // false once queue_pop() is called.
    bool overflowFlag;

    // Name for debugging purposes.
    char name[QUEUE_MAX_NAME_SIZE];
} queue_t;

// Allocates memory for the queue (the data* pointer) and initializes all
// parts of the data structure. Prints out an error message if malloc() fails
// and calls assert(false) to print-out line-number information and die.
// The queue is empty after initialization. To fill the queue with known
// values (e.g. zeros), call queue_overwritePush() up to queue_size() times.
void queue_init(queue_t *q, queue_size_t size, const char *name);

// Get the user-assigned name for the queue.
const char *queue_name(queue_t *q);

// Returns the capacity of the queue.
queue_size_t queue_size(queue_t *q);

// Returns true if the queue is full.
bool queue_full(queue_t *q);

// Returns true if the queue is empty.
bool queue_empty(queue_t *q);

// If the queue is not full, pushes a new element into the queue and clears the
// underflowFlag. IF the queue is full, set the overflowFlag, print an error
// message and DO NOT change the queue.
void queue_push(queue_t *q, queue_data_t value);

// If the queue is not empty, remove and return the oldest element in the queue.
// If the queue is empty, set the underflowFlag, print an error message, and DO
// NOT change the queue.
queue_data_t queue_pop(queue_t *q);

// If the queue is full, call queue_pop() and then call queue_push().
// If the queue is not full, just call queue_push().
void queue_overwritePush(queue_t *q, queue_data_t value);

// Provides random-access read capability to the queue.
// Low-valued indexes access older queue elements while higher-value indexes
// access newer elements (according to the order that they were added). Print a
// meaningful error message if an error condition is detected.
queue_data_t queue_readElementAt(queue_t *q, queue_index_t index);

// Returns a count of the elements currently contained in the queue.
queue_size_t queue_elementCount(queue_t *q);

// Returns true if an underflow has occurred (queue_pop() called on an empty
// queue).
bool queue_underflow(queue_t *q);

// Returns true if an overflow has occurred (queue_push() called on a full
// queue).
bool queue_overflow(queue_t *q);

// Frees the storage that you malloc'd before.
void queue_garbageCollect(queue_t *q);

#endif //LASER_TAG_QUEUE_H
