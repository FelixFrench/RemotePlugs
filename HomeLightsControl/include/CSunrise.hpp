#ifndef C_SUNRISE_HPP
#define C_SUNRISE_HPP

#include <stdlib.h>

class CSunrise{
public:

// This should be called once per day before the sunrise-sunset times for that day are used
static void Update(void);

// Returns the sunrise time as minutes since midnight.
static uint16_t GetSunrise(void){
    return mSunriseMins;
}

// Returns the sunrise time as minutes since midnight.
static uint16_t GetSunset(void){
    return mSunsetMins;
}

private:

// sunrise-sunset.org API request.
const static char* mSunriseAPI;

// The times of sunrise and sunset in minutes since midnight.
static uint16_t mSunriseMins, mSunsetMins;
};

#endif