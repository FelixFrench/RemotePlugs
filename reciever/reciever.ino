// The transmitter is the little square one, the reciever is the long rectangular one

// Programming:
// To restore factory defaults, unplug the socket from the mains, then press and hold the yellow button on the side
//  of the socket for 5s while plugging it back in.
// To pair a remote control to a socket, turn the socket on by pressing the yellow button briefly, then press and hold
//  the yellow button again until the socket turns off. The desired 'ON' key on the remote can then be pressed to pair
//  that row of keys on the remote to the socket.

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

enum COMMAND {
  COMMAND_ON,
  COMMAND_OFF,
  COMMAND_BOTH,
  COMMAND_UNKNOWN,
};

const char* CommandStrings[] = {"on", "off", "both", "unknown"};

void setup() {
  Serial.begin(115200);
  
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
}

void loop() {
  if (mySwitch.available()) {
    long code = mySwitch.getReceivedValue();
    Serial.print("Received ");
    Serial.print( code );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    Serial.print( mySwitch.getReceivedProtocol() );
    Serial.print(" Command: ");
    Serial.print(GetRow(code));
    Serial.print(" ");
    Serial.println(CommandStrings[GetCommand(code)]);
    mySwitch.resetAvailable();
  }
}

int GetRow(long code)
{
  switch (code) {
    case 1332531:
    case 1332540:
    case 1332543: return 0;
    
    case 1332675:
    case 1332684:
    case 1332687: return 1;
      
    case 1332995:
    case 1333004:
    case 1333007: return 2;
    
    case 1334531:
    case 1334540:
    case 1334543: return 3;
    
    case 1340675:
    case 1340684:
    case 1340687: return 4;
    
    default: return 5;
  }
}

COMMAND GetCommand(long code)
{
  switch (code) {
    case 1332531:
    case 1332675:
    case 1332995:
    case 1334531:
    case 1340675: return COMMAND_ON;
    
    case 1332540:
    case 1332684:
    case 1333004:
    case 1334540:
    case 1340684: return COMMAND_OFF;
    
    case 1332543:
    case 1332687:
    case 1333007:
    case 1334543:
    case 1340687: return COMMAND_BOTH;
    
    default: return COMMAND_UNKNOWN;
  }
}
