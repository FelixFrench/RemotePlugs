#include "CTransmitter.hpp"

// Initialise static member variables
RCSwitch CTransmitter::mySwitch = RCSwitch();

void CTransmitter::Setup(void){

    // Set up the transmitter and define the pulse length and number of repititions
    mySwitch.enableTransmit(digitalPinToInterrupt(PIN_TRANSMIT));
    mySwitch.setPulseLength(150);
    mySwitch.setRepeatTransmit(1);
}

bool CTransmitter::Background(void){

    // This will be set if a transmission is made
    bool didATransmit = false;

    // Get the next code that needs sending from the queue.
    CRemoteCodes CodeToSend = CTransmitQueue::Pop();

    // Transmit a code if required
    if (CodeToSend != CRemoteCodes::NONE){
        mySwitch.send(static_cast<uint32_t>(CodeToSend), 24);
        didATransmit = true;
    }

    return didATransmit;
}

void CTransmitter::EnqueueTransmission(CRemoteCodes code){
    CTransmitQueue::Push(code);
}