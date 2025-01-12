#include "CTransmitter.hpp"
#include <Arduino.h>

void CTransmitter::Setup(void)
{
    // Set up the transmitter and define the pulse length and number of repititions
    pinMode(PIN_TRANSMIT, OUTPUT);
}

bool CTransmitter::Background(void)
{
    // This will be set if a transmission is made
    bool didATransmit = false;

    // Get the next code that needs sending from the queue.
    CRemoteCodes CodeToSend = CTransmitQueue::Pop();

    // Transmit a code if required
    if (CodeToSend != CRemoteCodes::NONE){
        Send(CodeToSend, 24);
        didATransmit = true;
    }

    return didATransmit;
}

void CTransmitter::EnqueueTransmission(CRemoteCodes code)
{
    CTransmitQueue::Push(code);
}

// The Send and Transmit functions are based on the RCSwitch library: https://github.com/sui77/rc-switch
void CTransmitter::Send(CRemoteCodes code, int16_t length)
{
    // Send the code big-endian.
    for (int16_t i = length-1; i >= 0; i--) {
        if (static_cast<uint32_t>(code) & (1L << i))
            Transmit(ONE_HIGH, ONE_LOW);
        else
            Transmit(ZERO_HIGH, ZERO_LOW);
    }

    // Send the sync factor
    Transmit(SYNC_HIGH, SYNC_LOW);
}

void CTransmitter::Transmit(uint16_t High, uint16_t Low)
{
    // Transmit a high-low pulse.
    digitalWrite(PIN_TRANSMIT, HIGH);
    delayMicroseconds( PULSE_LENGTH * High);
    digitalWrite(PIN_TRANSMIT, LOW);
    delayMicroseconds( PULSE_LENGTH * Low);
}