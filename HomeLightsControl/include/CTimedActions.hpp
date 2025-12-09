#ifndef C_TIMED_ACTIONS_HPP
#define C_TIMED_ACTIONS_HPP

#include "time.h"

class CTimedActions {

public:
// Set up connection to the ntp server and do any backscanning
// Backscanning is looking for the last code that would have been sent - useful after a power outage when all plugs turn off.
static void Setup();

// Add any required actions for the current time to the transmit queue
static void Run();


private:

// Web address of the ntp server to use
const static char* mNTP_Server;

// TimeZone rule. See https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
const static char* mTimeZone;

// Returns true if a tm represents a time which is on Saturday or Sunday.
static inline bool isWeekend(tm currentTime){
    return (currentTime.tm_wday == 0) || (currentTime.tm_wday == 6);
}

// Returns the minutes since midnight for a tm struct
static inline int16_t minsSinceMidnight(tm currentTime){
    return currentTime.tm_hour * 60 + currentTime.tm_min;
}

// Get the epoch time. This will return once the time has been found from the ntp server.
static void getLocalEpochTime(time_t* pNow);

// Adds any bedroom lights action required at the current time to the queue. Returns true if an action was required
static bool BedroomLights(tm currentTime);

// Adds any garden lights action required at the current time to the queue. Returns true if an action was required
static bool GardenLights(tm currentTime);

// Adds any Christmas lights action required at the current time to the queue. Returns true if an action was required
static bool ChristmasLights(tm currentTime);

// Adds any Christmas tree action required at the current time to the queue. Returns true if an action was required
static bool ChristmasTree(tm currentTime);

};

#endif
