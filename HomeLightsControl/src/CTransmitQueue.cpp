#include "CTransmitQueue.hpp"

// Initialise static member variables
CRemoteCodes    CTransmitQueue::Queue[CTransmitQueue::QUEUE_SIZE] = {CRemoteCodes::NONE};
uint8_t     CTransmitQueue::HeadIndex = 0;
uint8_t     CTransmitQueue::TailIndex = 0;
uint16_t    CTransmitQueue::RemainingTransmits = 0;

void CTransmitQueue::Push(CRemoteCodes TransmitCode)
{
    // If the queue is full, do nothing
    if((HeadIndex == TailIndex) && (RemainingTransmits != 0)){
        return;
    }

    // Put the new queue item in the first element after the end of the queue.
    Queue[TailIndex] = TransmitCode;

    // Increment the tail index, setting it to zero if the end of the array is reached.
    if(++TailIndex == QUEUE_SIZE){
        TailIndex = 0;
    }

    // If the queue was previously empty, load RemainingTransmits
    if (RemainingTransmits == 0){
        RemainingTransmits = TRANSMIT_REPEATS;
    }
}

CRemoteCodes CTransmitQueue::Pop(void)
{
    // If the queue is empty, return none.
    if (RemainingTransmits == 0) {
        return CRemoteCodes::NONE;
    }

    CRemoteCodes CodeToReturn = Queue[HeadIndex];

    // If this was the last transmit of this code, move the head along
    if (--RemainingTransmits == 0) {

        // Increment the head index, setting it to zero if the end of the array is reached.
        if (++HeadIndex == QUEUE_SIZE) {
            HeadIndex = 0;
        }

        // If the head and tail indices are not equal (there are more elements in the queue)
        if (HeadIndex != TailIndex) {
            // Reload remaining transmits
            RemainingTransmits = TRANSMIT_REPEATS;
        }
    }

    return CodeToReturn;
}
