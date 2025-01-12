#ifndef C_TIMED_ACTIONS_HPP
#define C_TIMED_ACTIONS_HPP

#include "time.h"

class CTimedActions {

public:
// Set up connection to the ntp server and transmit the last signal for each timer
static void Setup();

// Add any required actions for the current time to the transmit queue
static void Run();


private:

// Web address of the ntp server to use
const static char* mNTP_Server;

// TimeZone rule. See https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
const static char* mTimeZone;

// Get the epoch time. This will return once the time has been found from the ntp server.
static void getLocalEpochTime(time_t* pNow);

// Adds any bedroom lights action required at the current time to the queue. Returns true if an action was required
static void BedroomLights(tm timeinfo);

// Adds any garden lights action required at the current time to the queue. Returns true if an action was required
static bool GardenLights(tm timeinfo);

};

#endif