#ifndef C_TRANSMITTER_HPP
#define C_TRANSMITTER_HPP

#include "stdlib.h"
#include "CRemoteCodes.hpp"
#include "CTransmitQueue.hpp"


class CTransmitter {

public:

// Set up the transmitter. Must be called before use.
static void Setup(void);

// Send a transmission if required. Returns true if a transmission was made.
static bool Background(void);

// Set up a transmission to be sent in the next NUM_REPEATS calls of Background
static void EnqueueTransmission(CRemoteCodes code);

private:
enum {
    PIN_TRANSMIT = 12,
    PULSE_LENGTH = 150,

    ZERO_HIGH = 1,
    ZERO_LOW = 3,
    ONE_HIGH = 3,
    ONE_LOW = 1,
    SYNC_HIGH = 1,
    SYNC_LOW = 31
};

// Send a given code. Length gives the number of bits the code uses. Code should be right-justified.
static void Send(CRemoteCodes code, int16_t length);

// Transmit a high-low pulse. High and Low determine the on and off times in units of PULSE_LENGTH * microsecond.
static void Transmit(uint16_t High, uint16_t Low);

};
#endif