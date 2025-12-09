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
    // TODO: Use <esp_sntp.h>
    //       sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED

    struct tm info;
    
    while (true) {

        // Store the current time
        time(pNow);

        // Convert pNow to a struct tm in local time
        localtime_r(pNow, &info);

        // Only return once NTP has synced with the current time
        if (info.tm_year > (2016 - 1900))
            return;
        
        // ESP32 delay
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

    // Transmit the last required code for each switch
    // This means that at turn-on or after a power cut, all plugs will automatically
    //  be returned to the correct state
    bool gardenLightsDone = false;
    bool christmasLightsDone = false;
    bool christmasTreeDone = false;
    tm simulatedStructTime;

    while (true){

        // Convert the epoch time (time_t) to a tm struct
        localtime_r(&simulatedEpochTime, &simulatedStructTime);

        // Try each switch which hasn't yet found its last transmission
        if (!gardenLightsDone)
            gardenLightsDone = GardenLights(simulatedStructTime);
        if (!christmasLightsDone)
            christmasLightsDone = ChristmasLights(simulatedStructTime);
        if (!christmasTreeDone)
            christmasTreeDone = ChristmasTree(simulatedStructTime);

        // If all switches have now had their last transmission sent, break
        if (gardenLightsDone && christmasLightsDone && christmasTreeDone)
            break;

        // Otherwise, there are more transmissions to be found and sent, so move the simulated time back a minute.
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
    ChristmasLights(timeinfo);
    ChristmasTree(timeinfo);


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

bool CTimedActions::BedroomLights(tm currentTime)
{
    // The bedroom lights turn on at 06:55 and off at 07:35 on weekdays only

    // Times in minutes from midnight
    enum {
        ON_MINS = 415,
        OFF_MINS = 455
    };

    // If it is the weekend, no action is required.
    if (isWeekend(currentTime))
        return false;

    // If any action is required during the current minute, this will be set to true.
    bool actionRequired = false;

    uint16_t currentMins = minsSinceMidnight(currentTime);

    if(currentMins == ON_MINS){
        CTransmitQueue::Push(CRemoteCodes::NEW_0_ON);
        actionRequired = true;
    }

    else if (currentMins == OFF_MINS){
        CTransmitQueue::Push(CRemoteCodes::NEW_0_OFF);
        actionRequired = true;
    }

    if (!actionRequired) Serial.printf("No bedroom lights action at %d mins \n", currentMins);

    return actionRequired;
}

bool CTimedActions::GardenLights(tm currentTime)
{
    // If sunrise is after 07:20 then
    //   the garden lights turn on at 07:20 and off at sunrise
    // If sunset is before 23:30 then
    //   the garden lights turn on at sunset and off at 23:30

    // Times in minutes from midnight
    enum {
        MORNING_ON_MINS = 440,
        NIGHT_OFF_MINS = 1410
    };

    // If any action is required during the current minute, this will be set to true.
    bool actionRequired = false;

    uint16_t SunriseMins = CSunrise::GetSunrise();
    uint16_t SunsetMins = CSunrise::GetSunset();
    uint16_t currentMins = minsSinceMidnight(currentTime);

    // If sunrise is after the morning on time
    if (MORNING_ON_MINS < SunriseMins) {

        // Turn on at the scheduled time
        if (currentMins == MORNING_ON_MINS) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
            Serial.println("Garden lights morning on");
            actionRequired = true;
        }
        // Turn off at sunrise
        else if (currentMins == SunriseMins) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
            actionRequired = true;
            Serial.println("Garden lights sunrise off");
        }
    }

    // If sunset is before the night off time
    if (SunsetMins < NIGHT_OFF_MINS) {

        // Turn on at sunset
        if (currentMins == SunsetMins) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
            actionRequired = true;
            Serial.println("Garden lights sunset on");
        }
        // Turn off at the scheduled time
        else if (currentMins == NIGHT_OFF_MINS) {
            CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
            actionRequired = true;
            Serial.println("Garden lights night off");
        }
    }

    if (!actionRequired) Serial.printf("No garden lights action at %d mins \n", currentMins);

    return actionRequired;
}

bool CTimedActions::ChristmasLights(tm currentTime)
{
    // If sunset is before 23:30 then
    //   the garden lights turn on at sunset and off at 23:30

    // Times in minutes from midnight
    enum {
        NIGHT_OFF_MINS = 1410
    };

    // If any action is required during the current minute, this will be set to true.
    bool actionRequired = false;

    uint16_t SunsetMins = CSunrise::GetSunset();
    uint16_t currentMins = minsSinceMidnight(currentTime);

    // If sunset is before the night off time
    if (SunsetMins < NIGHT_OFF_MINS) {

        // Turn on at sunset
        if (currentMins == SunsetMins) {
            CTransmitQueue::Push(CRemoteCodes::NEW_3_ON);
            actionRequired = true;
            Serial.println("Christmas lights sunset on");
        }
        // Turn off at the scheduled time
        else if (currentMins == NIGHT_OFF_MINS) {
            CTransmitQueue::Push(CRemoteCodes::NEW_3_OFF);
            actionRequired = true;
            Serial.println("Christmas lights night off");
        }
    }

    if (!actionRequired) Serial.printf("No Christmas lights action at %d mins \n", currentMins);

    return actionRequired;
}

bool CTimedActions::ChristmasTree(tm currentTime)
{
    //  if weekend
    //      turn on at 07:20
    //      turn off at 23:30
    //  else
    //      turn on at 07:20
    //      turn off at max(08:30, sunrise)
    //      turn on at min(17:00, sunset)
    //      turn off at 23:30

    // Times in minutes from midnight
    enum {
        MORNING_ON_MINS = 440,
        WEEKDAY_MORNING_OFF_MINS = 510,
        WEEKDAY_EVENING_ON_MINS = 1020,
        NIGHT_OFF_MINS = 1410
    };

    // If any action is required during the current minute, this will be set to true.
    bool actionRequired = false;

    uint16_t currentMins = minsSinceMidnight(currentTime);

    // Turn on at the scheduled time
    if (currentMins == MORNING_ON_MINS) {
        CTransmitQueue::Push(CRemoteCodes::NEW_4_ON);
        Serial.println("Christmas tree morning on");
        actionRequired = true;
    }

    // Turn off at the scheduled time
    else if (currentMins == NIGHT_OFF_MINS) {
        CTransmitQueue::Push(CRemoteCodes::NEW_4_OFF);
        actionRequired = true;
        Serial.println("Garden lights night off");
    }

    // If today is a weekday
    else if (!isWeekend(currentTime)){
        
        // On weekdays, turn off at the later of sunrise or the set time
        uint16_t sunriseMins = CSunrise::GetSunrise();
        uint16_t morningOffMins = WEEKDAY_MORNING_OFF_MINS > sunriseMins ? WEEKDAY_MORNING_OFF_MINS : sunriseMins;

        // On weekdays, turn on at the later of sunset or the set time
        uint16_t sunsetMins = CSunrise::GetSunset();
        uint16_t eveningOnMins = WEEKDAY_EVENING_ON_MINS < sunsetMins ? WEEKDAY_EVENING_ON_MINS : sunsetMins;

        if (currentMins == morningOffMins) {
            CTransmitQueue::Push(CRemoteCodes::NEW_4_OFF);
            actionRequired = true;
            Serial.println("Christmas tree weekday morning off");
        }
        else if (currentMins == eveningOnMins) {
            CTransmitQueue::Push(CRemoteCodes::NEW_4_ON);
            actionRequired = true;
            Serial.println("Christmas tree weekday morning on");
        }
    }

    if (!actionRequired) Serial.printf("No Christmas tree action at %d mins \n", currentMins);

    return actionRequired;
}
