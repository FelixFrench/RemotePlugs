#include "CTransmitter.hpp"

// Initialise static member variables
RCSwitch CTransmitter::mySwitch = RCSwitch();
CRemoteCodes CTransmitter::codeToSend = CRemoteCodes::NONE;
uint16_t CTransmitter::repeatsRemaining = 0;


void CTransmitter::Setup(void){

    // Set up the transmitter and define the pulse length and number of repititions
    mySwitch.enableTransmit(digitalPinToInterrupt(PIN_TRANSMIT));
    mySwitch.setPulseLength(150);
    mySwitch.setRepeatTransmit(1);
}

bool CTransmitter::Background(void){

    // This will be set if a transmission is made
    bool didATransmit = false;

    // Transmit a code if required
    if (repeatsRemaining > 0) {
        mySwitch.send(static_cast<uint32_t>(codeToSend), 24);
        repeatsRemaining--;
        didATransmit = true;
    }

    return didATransmit;
}

void CTransmitter::StartTransmitting(CRemoteCodes code){
    
    codeToSend = code;
    repeatsRemaining = NUM_REPEATS;
}