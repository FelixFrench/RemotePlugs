#include "TransmitQueue.hpp"

// Initialise static member variables
uint32_t    TransmitQueue::Queue[TransmitQueue::QUEUE_SIZE] = {0};
uint8_t     TransmitQueue::HeadIndex;
uint8_t     TransmitQueue::TailIndex;
uint16_t    TransmitQueue::RemainingTransmits = TransmitQueue::TRANSMIT_REPEATS;

void TransmitQueue::Push(uint32_t TransmitCode)
{
    // If the queue is full, do nothing
    if(HeadIndex == (TailIndex + 1) % QUEUE_SIZE){
        return;
    }

    // Put the new queue item in the first element after the end of the queue.
    Queue[TailIndex] = TransmitCode;

    // Increment the tail index, setting it to zero if the end of the array is reached.
    if(++TailIndex == QUEUE_SIZE){
        TailIndex = 0;
    }
}

uint32_t TransmitQueue::Pop(void)
{
    // If the queue is empty, return zero.
    if(HeadIndex == TailIndex){
        return 0;
    }

    uint32_t CodeToReturn = Queue[HeadIndex];

    // If this was the last transmit of this code, move the head along
    if(--RemainingTransmits == 0){
        
        // Reload remaining transmits
        RemainingTransmits = TRANSMIT_REPEATS;

        // Increment the head index, setting it to zero if the end of the array is reached.
        if(++HeadIndex == QUEUE_SIZE){
            HeadIndex = 0;
        }
    }

    return CodeToReturn;
}
