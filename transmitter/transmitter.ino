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

// The codes transmitted by the old remote.
enum OldCodeValues {
  OLD_CODE_0_ON = 1332531,
  OLD_CODE_0_OFF = 1332540, 
  OLD_CODE_1_ON = 1332675,
  OLD_CODE_1_OFF = 1332684, 
  OLD_CODE_2_ON = 1332995,
  OLD_CODE_2_OFF = 1333004, 
  OLD_CODE_3_ON = 1334531,
  OLD_CODE_3_OFF = 1334540, 
  OLD_CODE_4_ON = 1340675,
  OLD_CODE_4_OFF = 1340684, 
};

// The codes transmitted by the new remote.
enum NewCodeValues {
  NEW_CODE_0_ON = 15404668,
  NEW_CODE_0_OFF = 15404660,
  NEW_CODE_1_ON = 15404666,
  NEW_CODE_1_OFF = 15404658,
  NEW_CODE_2_ON = 15404665,
  NEW_CODE_2_OFF = 15404657,
  NEW_CODE_3_ON = 15404669,
  NEW_CODE_3_OFF = 15404661,
  NEW_CODE_4_ON = 15404667,
  NEW_CODE_4_OFF = 15404659,
};

// Function to work out whether a given UTC datetime is during british summer time.
bool IsBST(Ds1302::DateTime dt);

// Functions to send any signal that is required at the current date & time.
bool CheckForGardenEvent(Ds1302::DateTime dt);
bool CheckForChristmasEvent(Ds1302::DateTime dt);
void CheckForBedroomEvent(Ds1302::DateTime dt);

// Globals
RCSwitch mySwitch = RCSwitch();
Ds1302 rtc(PIN_RTC_RST, PIN_RTC_DAT, PIN_RTC_CLK);
int SunriseMins, SunsetMins;
bool isBST;
bool SignalSentSinceReset = false;

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

  // The current day of the week with daylight saving time applied.
  static uint8_t dstDow;

  // Update whether its daylight saving time and the sunrise/sunset times once per hour. (Also runs on the first loop.)
  static uint8_t last_hour = 0;
  if (UTC.hour != last_hour) {
    last_hour = UTC.hour;
    isBST = IsBST(UTC);

    // Get today's sunrise and sunset times. Note this changes when daylight saving time starts/stops, so must be refreshed hourly.
    static Dusk2Dawn d2d(Latitude, Longitude, Timezone);
    SunriseMins = d2d.sunrise(UTC.year, UTC.month, UTC.day, isBST);
    SunsetMins = d2d.sunset(UTC.year, UTC.month, UTC.day, isBST);

    // Get strings of the new sunrise and sunset times and print them.
    char sunriseStr[] = "00:00", sunsetStr[] = "00:00";
    Dusk2Dawn::min2str(sunriseStr, SunriseMins);
    Dusk2Dawn::min2str(sunsetStr, SunsetMins);

    Serial.print("NEW HOUR: Sunrise: ");
    Serial.print(sunriseStr);
    Serial.print(", sunset: ");
    Serial.println(sunsetStr);

    // Work out what day of the week it is with daylight saving time applied.
    dstDow = UTC.dow;

    // If british summer time is active and the UTC time is 23:00 or later, the day of the week must be incremented.
    if (isBST && UTC.hour == 23) {

      // Return to Monday once beyond Sunday.
      if (++dstDow > 7) {
        dstDow = 1;
      }
    }
  }

  // The current minutes since midnight, with daylight saving time applied.
  static uint16_t dstMins;

  // Check for events once per minute
  static uint8_t last_minute = 0;
  if (UTC.minute != last_minute) {
    last_minute = UTC.minute;

    // Get the current minutes since midnight in GMT or BST as required.
    dstMins = 60*((UTC.hour + (isBST ? 1 : 0)) % 24) + UTC.minute;

    CheckForGardenEvent(dstMins);
    CheckForChristmasEvent(dstMins);
    CheckForBedroomEvent(dstDow, dstMins);
  }

  // If no signal has been sent since the last processor reset, then the lights may be in the wrong state. Find the last update event.
  if (!SignalSentSinceReset) {

    // Note: This may miss the last event if it was between 01:00 and 02:00 and daylight savings time has ended within the last hour.

    // Start at the current time.
    uint16_t SimulatedMins = dstMins;

    // Keep decrementing the time until a time at which an event occurs is found.
    while (!CheckForGardenEvent(SimulatedMins)) {
    
      // Decrement SimulatedMins unless the current time is already zero, in which case reset to 23:59.
      if (SimulatedMins-- == 0) {
        SimulatedMins = 23 * 60 + 59;
      }
    }

   // There is no control of the bedroom lights here as it is more likely to just end up turning the lights off on a Sunday while they're in use than actually being useful.
   // The only scenario in which this could be a problem is if the bedroom lights come on at 06:15 as planned then there is a power cut between 06:16 and us waking up,
   //  which would result in the lights being off when we wake up. At that point it would be an act of God and clearly we were just meant to have a lie in.

    SignalSentSinceReset = true;
  }


  // Print the time once per second. (Also runs on the first loop.)
  static uint8_t last_second = 0;
  if (UTC.second != last_second) {
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

// This function looks to see if the garden lights need to turn on or off at the current time.
// If a command is sent then true is returned, otherwise false is returned.
bool CheckForGardenEvent(uint16_t NowMins) {
  enum {
    MORNING_ON = 6*60 + 30,
    NIGHT_OFF = 23*60 + 30
  };

  // If sunrise is after 06:30
  if (SunriseMins > MORNING_ON) {

    // Turn on at 06:30 
    if (NowMins == MORNING_ON) {
      Serial.print("Garden morning turn on");
      mySwitch.send(NEW_CODE_2_ON, 24);
      Serial.println(" complete");
      return true;
    }

    // Turn off at sunrise
    if (NowMins == SunriseMins) {
      Serial.print("Garden morning turn off");
      mySwitch.send(NEW_CODE_2_OFF, 24);
      Serial.println(" complete");
      return true;
    }
  }

  // If sunset is before 23:30
  if (SunsetMins < NIGHT_OFF) {

    // Turn on at sunset
    if (NowMins == SunsetMins) {
      Serial.print("Garden evening turn on");
      mySwitch.send(NEW_CODE_2_ON, 24);
      Serial.println(" complete");
      return true;
    }

    // Turn off at 23:30
    if (NowMins == NIGHT_OFF) {
      Serial.print("Garden evening turn off");
      mySwitch.send(NEW_CODE_2_OFF, 24);
      Serial.println(" complete");
      return true;
    }
  }

  Serial.print(NowMins);
  Serial.println(": No garden event");

  return false;
}

// This function looks to see if the Christmas lights need to turn on or off at the current time.
// If a command is sent then true is returned, otherwise false is returned.
bool CheckForChristmasEvent(uint16_t NowMins) {
  enum {
    NIGHT_OFF = 22*60,
  };

  // If sunset is before 23:30
  if (SunsetMins < NIGHT_OFF) {

    // Turn on at sunset
    if (NowMins == SunsetMins) {
      Serial.print("Christmas evening turn on");
      mySwitch.send(NEW_CODE_4_ON, 24);
      Serial.println(" complete");
      return true;
    }

    // Turn off at 23:30
    if (NowMins == NIGHT_OFF) {
      Serial.print("Christmas evening turn off");
      mySwitch.send(NEW_CODE_4_OFF, 24);
      Serial.println(" complete");
      return true;
    }
  }

  Serial.print(NowMins);
  Serial.println(": No Christmas event");

  return false;
}

// This function looks to see whether the bedroom lights need to turn on at the current time on the curent day of the week.
void CheckForBedroomEvent(uint8_t dow, uint16_t NowMins) {

  enum {
    MON_THURS_ON = 6 * 60 + 15,
    MON_THURS_OFF = 6 * 60 + 40,
    FRI_ON = 6 * 60 + 15,
    FRI_OFF = 6 * 60 + 40,
  };

  // On Monday - Thursday
  if (dow <= 4 ) {
    
    // Turn on at 06:15
    if (NowMins == MON_THURS_ON) {
      Serial.print("Bedroom Mon-Thurs turn on");
      mySwitch.send(NEW_CODE_0_ON, 24);
      Serial.println(" complete");
    }

    // Turn off at 06:40
    if (NowMins == MON_THURS_OFF) {
      Serial.print("Bedroom Mon-Thurs turn off");
      mySwitch.send(NEW_CODE_0_OFF, 24);
      Serial.println(" complete");
    }
  }

  // On Friday
  if (dow == 5) {
    
    // Turn on at 07:25
    if (NowMins == FRI_ON) {
      Serial.print("Bedroom Friday turn on");
      mySwitch.send(NEW_CODE_0_ON, 24);
      Serial.println(" complete");
    }

    // Turn off at 07:50
    if (NowMins == FRI_OFF) {
      Serial.print("Bedroom Friday turn off");
      mySwitch.send(NEW_CODE_0_OFF, 24);
      Serial.println(" complete");
    }
  }

  Serial.print(NowMins);
  Serial.println(": No bedroom event");
}