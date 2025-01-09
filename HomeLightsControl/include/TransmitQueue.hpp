#include "stdlib.h"

class TransmitQueue
{
private:

enum {
MAX_QUEUE_LENGTH = 5,
QUEUE_SIZE = MAX_QUEUE_LENGTH + 1,
TRANSMIT_REPEATS = 100,
};

// The queue of codes to send
static uint32_t Queue[QUEUE_SIZE];

// The head and tail indexes of the queue. Both have range 0 to QUEUE_LENGTH-1.
// If equal the queue is empty, if tail=(head-1)%QUEUE_SIZE the queue is full
// If the queue is [x, 1, 2, 3, x, x] then HeadIndex = 1, TailIndex = 4
// If the queue is [3, 4, 5, x, 1, 2] then HeadIndex = 4, TailIndex = 3 (full)
static uint8_t HeadIndex, TailIndex;

// The number more times to transmit the code at the head of the queue (if any)
static uint16_t RemainingTransmits;

public:
    // Add an item to the queue
    static void Push(uint32_t TransmitCode);

    // Get the next code to transmit
    static uint32_t Pop(void);
};


