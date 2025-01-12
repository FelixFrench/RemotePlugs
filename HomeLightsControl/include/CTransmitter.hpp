#ifndef C_TRANSMITTER_HPP
#define C_TRANSMITTER_HPP

#include "stdlib.h"
#include <RCSwitch.h>
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
enum{
  // The pin which the transmitter is connected to
  PIN_TRANSMIT = 12
};

// The transmitter
static RCSwitch mySwitch;

};
#endif