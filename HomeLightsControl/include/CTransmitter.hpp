#ifndef C_TRANSMITTER_HPP
#define C_TRANSMITTER_HPP

#include "stdlib.h"
#include <RCSwitch.h>
#include "CRemoteCodes.hpp"


class CTransmitter {

public:

// Set up the transmitter. Must be called before use.
static void Setup(void);

// Send a transmission if required. Returns true if a transmission was made.
static bool Background(void);

// Set up a transmission to be sent in the next NUM_REPEATS calls of Background
static void StartTransmitting(CRemoteCodes code);

private:
enum{
  // The number of times to transmit any code.
  NUM_REPEATS = 100,

  // The pin which the transmitter is connected to
  PIN_TRANSMIT = 12
};

// The transmitter
static RCSwitch mySwitch;

// The code which is currently being transitted, or was last transmitted.
static CRemoteCodes codeToSend;

// The number more times to transit codeToSend.
static uint16_t repeatsRemaining;

};
#endif