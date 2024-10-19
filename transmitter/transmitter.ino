// The transmitter is the little square PCB, the reciever is the long rectangular PCB
#include <RCSwitch.h>
#include <Ds1302.h>
#include <Dusk2Dawn.h>

enum PIN {
  PIN_TOGGLE_BUTTON = 2,
  PIN_LED = 3,
  
  PIN_RTC_RST = 5,
  PIN_RTC_DAT = 7,
  PIN_RTC_CLK = 6,

  PIN_TRANSMITTER = 10,
};

const int Timezone = 0; // GMT
const double Latitude = 52.711, Longitude = -2.766; // SY3 8NL

// The codes transmitted by the remote.
const long CodeValues[10] = { // rows 0->4, on then off
  1332531, 1332540, 
  1332675, 1332684, 
  1332995, 1333004, 
  1334531, 1334540, 
  1340675, 1340684, 
};

enum {
  CHANNEL = 3, // The row of buttons on the remote to simulate.
  ON_CHANNEL = CHANNEL * 2,
  OFF_CHANNEL = CHANNEL * 2 + 1,
};

enum TIME {
  TIME_NIGHT_OFF = 23*60 + 30, // Turn off at midnight after sunset
  TIME_MORNING_ON = 6*60 + 30, // Turn on at 06:30 before sunrise
};

enum STATE {
  STATE_UNKNOWN,
  STATE_TURN_ON,
  STATE_ON,
  STATE_TURN_OFF,
  STATE_OFF,
};

// Function to work out whether a given UTC datetime is during british summer time.
bool IsBST(Ds1302::DateTime dt);

// Globals
RCSwitch mySwitch = RCSwitch();
Ds1302 rtc(PIN_RTC_RST, PIN_RTC_DAT, PIN_RTC_CLK);

void setup() {
  // Set up the transmitter and define the pulse length and number of repititions
  mySwitch.enableTransmit(PIN_TRANSMITTER);
  mySwitch.setPulseLength(150);
  mySwitch.setRepeatTransmit(100);

  // Initialise the rtc
  rtc.init();

  // Set up the pins for the button and LED
  pinMode(PIN_TOGGLE_BUTTON, INPUT);
  pinMode(PIN_LED, OUTPUT);

  Serial.begin(115200);
}

void loop() {

  // Get the current time.
  Ds1302::DateTime UTC;
  rtc.getDateTime(&UTC);

  // Update whether its daylight saving time once per hour. (Also runs on the first loop.)
  static bool isBST;
  static uint8_t last_hour = 0;
  if(UTC.hour != last_hour){
    last_hour = UTC.hour;
    isBST = IsBST(UTC);
  }

  // Get the current minutes since midnight in GMT or BST as required.
  uint16_t dstMins = 60*((UTC.hour + (isBST ? 1 : 0)) % 24) + UTC.minute;

  // Update the sunset time once per day. (Also runs on the first loop.)
  static int sunriseMins, sunsetMins;
  static uint8_t last_day = 0;
  if ( UTC.day != last_day ){
    last_day = UTC.day;

    // Get today's sunrise and sunset times.
    static Dusk2Dawn d2d(Latitude, Longitude, Timezone);
    sunriseMins = d2d.sunrise(UTC.year, UTC.month, UTC.day, isBST);
    sunsetMins = d2d.sunset(UTC.year, UTC.month, UTC.day, isBST);

    // Get strings of the new sunrise and sunset times and print them.
    char sunriseStr[] = "00:00", sunsetStr[] = "00:00";
    Dusk2Dawn::min2str(sunriseStr, sunriseMins);
    Dusk2Dawn::min2str(sunsetStr, sunsetMins);

    Serial.print("NEW DAY: Sunrise: ");
    Serial.print(sunriseStr);
    Serial.print(", sunset: ");
    Serial.println(sunsetStr);
  }

  // Print the time once per second. (Also runs on the first loop.)
  static uint8_t last_second = 0;
  if ( UTC.second != last_second )
  {
    last_second = UTC.second;

    // Print the time
    Serial.print("20");
    Serial.print(UTC.year);    // 00-99
    Serial.print('.');
    if (UTC.month < 10) Serial.print('0');
    Serial.print(UTC.month);   // 01-12
    Serial.print('.');
    if (UTC.day < 10) Serial.print('0');
    Serial.print(UTC.day);     // 01-31
    Serial.print(' ');
    if ( UTC.hour < 10 ) Serial.print('0');
    Serial.print(UTC.hour);    // 00-23
    Serial.print(':');
    if ( UTC.minute < 10 ) Serial.print('0');
    Serial.print(UTC.minute);  // 00-59
    Serial.print(':');
    if ( UTC.second < 10 ) Serial.print('0');
    Serial.print(UTC.second);  // 00-59
    Serial.print(" UTC ");
    Serial.print(dstMins);
    Serial.println();
  }

  // State machine.
  static STATE mState = STATE_UNKNOWN;
  switch(mState){
  default: { // STATE_UNKNOWN
    
    // This code only runs after a reset: Figure out whether the lights should be on right now.
    if ((sunsetMins < dstMins && dstMins < TIME_NIGHT_OFF) || (TIME_MORNING_ON < dstMins && dstMins < sunriseMins)){
      mState = STATE_TURN_ON;
    } else {
      mState = STATE_TURN_OFF;
    }
    break;
  }
  case STATE_TURN_ON:{

    // Now that the LED has been turned on, spend a while transmitting ON.
    Serial.print("Turn on");
    mySwitch.send(CodeValues[ON_CHANNEL], 24);
    Serial.println(" complete");
    mState = STATE_ON;
    break;
  }
  case STATE_ON: {

    bool ButtonPressed = digitalRead(PIN_TOGGLE_BUTTON);
    if (ButtonPressed) Serial.println("BUTTON");
  
    // Turn off at sunrise, the evening turn-off time or on a button press.
    // There is no checking that the morning or evening start times are before the end times, the
    //  off signal is always sent. This means the lights will be turned off even if they were turned on
    //  by the toggle button or a different controller.
    if ( dstMins == sunriseMins || dstMins == TIME_NIGHT_OFF || ButtonPressed ) {
      mState = STATE_TURN_OFF;
    }
    break;
  }
  case STATE_TURN_OFF:{

    // Now that the LED has been turned off, spend a while transmitting OFF.
    Serial.print("Turn off");
    mySwitch.send(CodeValues[OFF_CHANNEL], 24);
    Serial.println(" complete");
    mState = STATE_OFF;
    break;
  }
  case STATE_OFF: {

    bool ButtonPressed = digitalRead(PIN_TOGGLE_BUTTON);
    if (ButtonPressed) Serial.println("BUTTON");

    // A morning turn-on is only required if the turn-on time is before sunrise
    bool MorningTurnOn = (dstMins == TIME_MORNING_ON) && (TIME_MORNING_ON < sunriseMins);

    // An evening turn-on is only required if the turn-off time is after sunset
    bool EveningTurnOn = (dstMins == sunsetMins) && (TIME_NIGHT_OFF > sunsetMins);
    
    // Turn on at the morning turn-on time, at sunset or on a button press
    if ( MorningTurnOn || EveningTurnOn || ButtonPressed ){
      mState = STATE_TURN_ON;
    }
    break;
  }
  }   

  // The LED is on if the lights are currently on or turning on.
  digitalWrite(PIN_LED, (mState == STATE_TURN_ON) || (mState == STATE_ON)); 
}

// Returns true if dt is within British Summer Time
// dt must be UTC
bool IsBST(Ds1302::DateTime dt){
  // BST starts at 01:00 UTC on the last Sunday in march
  // BST ends at 01:00 UTC on the last Sunday in October

  const uint8_t DaysInMarchAndOctober = 31;

  // If the month is before March or after October, return false
  if ( dt.month < 3 || 10 < dt.month ) return false;

  // If the month is after March and before October, return true
  if ( 3 < dt.month && dt.month < 10 ) return true;

  // Work out the day of the last Sunday in the current month, which must be March or October.
  // 1. Work out the number of days left in the month
  // 2. Add dow to get the number of days from last Sunday until month end
  // 3. Mod 7 to get the number of days from the last Sunday in the month until month end
  // 4. Subtract that from 31 to get the day of the last Sunday.
  uint8_t DayOfLastSunday = 31 - (31 - dt.day + dt.dow)%7;

  // The hour in the month at the start of which daylight savings time starts/ends.
  uint16_t changeHourInMonth = DayOfLastSunday * 24 + 1;

  // The hour in the month of dt.
  uint16_t dtHourInMonth = dt.day * 24 + dt.hour;

  // If the month is March, it's BST if dt is after the change time.
  if (dt.month == 3){
    return (dtHourInMonth >= changeHourInMonth);
    
  // If the month is October, it's BST if dt is before the change time.
  } else {
    return (dtHourInMonth < changeHourInMonth);
  }
}
