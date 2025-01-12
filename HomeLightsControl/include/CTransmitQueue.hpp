#ifndef C_TRANSMIT_QUEUE_HPP
#define C_TRANSMIT_QUEUE_HPP
#include "stdlib.h"
#include "CRemoteCodes.hpp"

class CTransmitQueue
{
public:
    // Add an item to the queue
    static void Push(CRemoteCodes TransmitCode);

    // Get the next code to transmit
    static CRemoteCodes Pop(void);

private:

enum {
    QUEUE_SIZE = 50,
    TRANSMIT_REPEATS = 50,
};

// The queue of codes to send
static CRemoteCodes Queue[QUEUE_SIZE];

// The head and tail indices of the queue. Both have range 0 to QUEUE_SIZE-1.
// The tail index is actually the element after the last item in the queue.
// If equal the queue is either empty or full. RemainingTransmits determines which one it is.
// If the queue is [x, 1, 2, 3, x, x] then HeadIndex = 1, TailIndex = 4
// If the queue is [3, 4, 5, 6, 1, 2] then HeadIndex = 4, TailIndex = 4 (full)
static uint8_t HeadIndex, TailIndex;

// The number more times to transmit the code at the head of the queue.
// This is always non-zero when there is anything in the queue. It is only zero when the queue is empty
static uint16_t RemainingTransmits;
};

#endif
