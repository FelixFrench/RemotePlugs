#include "WiFi.h"
#include "CTimedActions.hpp"
#include "CTransmitQueue.hpp"
#include "CRemoteCodes.hpp"
#include "NetCreds.h"
#include "CSunrise.hpp"

// Initialise static member variables
const char* CTimedActions::mNTP_Server = "pool.ntp.org";
// TimeZone rule for Europe/London including daylight adjustment rules.
// From https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
const char* CTimedActions::mTimeZone = "GMT0BST,M3.5.0/1,M10.5.0";

void CTimedActions::getLocalEpochTime(time_t* pNow)
{   
    struct tm info;
    
    while (true) {
        time(pNow);
        localtime_r(pNow, &info);
        if (info.tm_year > (2016 - 1900))
            return;

        delay(10);
    }
}

void CTimedActions::Setup()
{
    // Get the time
    configTzTime(mTimeZone, mNTP_Server);
    time_t simulatedEpochTime = 0;
    getLocalEpochTime(&simulatedEpochTime);

    Serial.println("Got the time");

    // Update the sunrise and sunset times
    CSunrise::Update();

    // Transmit the last required garden code
    bool transmissionComplete = false;
    tm simulatedStructTime;

    while (!transmissionComplete){

        // Convert the epoch time (time_t) to a tm struct
        localtime_r(&simulatedEpochTime, &simulatedStructTime);

        // If a transmission to send has been found, break.
        if (GardenLights(simulatedStructTime))
            transmissionComplete = true;

        // Otherwise, if the last transmission hasn't been found yet, go back a minute.
        else
            simulatedEpochTime -= 60;
    }

    Serial.println("Backscanning complete");

}

void CTimedActions::Run()
{   
    tm timeinfo;
    // Get the time
    getLocalTime(&timeinfo);

    // Second actions
    // ==============
    
    // If timed actions have already been run this second just return.
    uint8_t nowSeconds = timeinfo.tm_sec;
    static uint8_t lastSeconds = 0;
    if (nowSeconds == lastSeconds) {
        return;
    }
    lastSeconds = nowSeconds;

    // Print the time
    Serial.println(&timeinfo, "%a, %d %b %Y %H:%M:%S");


    // Minute actions
    // ==============

    // If timed actions have already been run this minute, return now.
    uint8_t nowMinute = timeinfo.tm_min;
    static uint8_t lastMinute = 0;
    if (nowMinute == lastMinute) {
        return;
    }
    lastMinute = nowMinute;

    BedroomLights(timeinfo);
    GardenLights(timeinfo);


    // Day actions
    // ==============

    // If timed actions have already been run today, return now.
    uint8_t nowDay = timeinfo.tm_mday;
    static uint8_t lastDay = 0;
    if (nowDay == lastDay) {
        return;
    }
    lastDay = nowDay;

    // Update the sunrise and sunset times
    CSunrise::Update();
}

void CTimedActions::BedroomLights(tm currentTime)
{
    // The bedroom lights turn on at 06:15 and off at 06:40 on weekdays only

    // Times in minutes from midnight
    enum {
        ON_MINS = 375,
        OFF_MINS = 400
    };

    // If it is the weekend, no action is required.
    if (currentTime.tm_wday == 0 || currentTime.tm_wday == 6)
        return;

    uint16_t currentMins = currentTime.tm_hour * 60 + currentTime.tm_min;

    if(currentMins == ON_MINS)
        CTransmitQueue::Push(CRemoteCodes::NEW_0_ON);

    if (currentMins == OFF_MINS)
        CTransmitQueue::Push(CRemoteCodes::NEW_0_OFF);
}

bool CTimedActions::GardenLights(tm currentTime)
{
    // If sunrise is after 07:00 then
    //   the garden lights turn on at 07:00 and off at sunrise
    // If sunset is before 23:30 then
    //   the garden lights turn on at sunset and off at 23:30

    // Times in minutes from midnight
    enum {
        MORNING_ON_MINS = 420,
        NIGHT_OFF_MINS = 1410
    };

    // If any action is required during the current minute, this will be set to true.
    bool ActionRequired = false;

    uint16_t SunriseMins = CSunrise::GetSunrise();
    uint16_t SunsetMins = CSunrise::GetSunset();
    int16_t currentMins = currentTime.tm_hour * 60 + currentTime.tm_min;

    // If sunrise is after the morning on time
    if (MORNING_ON_MINS < SunriseMins) {

        // Turn on at the scheduled time
        if (currentMins == MORNING_ON_MINS) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
            Serial.println("Garden lights morning on");
            ActionRequired = true;
        }
        // Turn off at sunrise
        else if (currentMins == SunriseMins) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
            ActionRequired = true;
            Serial.println("Garden lights sunrise off");
        }
    }

    // If sunset is before the night off time
    if (SunsetMins < NIGHT_OFF_MINS) {

        // Turn on at sunset
        if (currentMins == SunsetMins) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
            ActionRequired = true;
            Serial.println("Garden lights sunset on");
        }
        // Turn off at the scheduled time
        else if (currentMins == NIGHT_OFF_MINS) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
            ActionRequired = true;
            Serial.println("Garden lights night off");
        }
    }

    if (!ActionRequired) Serial.printf("No garden lights action at %d mins \n", currentMins);

    return ActionRequired;
}
