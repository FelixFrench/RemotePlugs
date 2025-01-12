#include "CSunrise.hpp"
#include "NetCreds.h"
#include "ArduinoJson.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* CSunrise::mSunriseAPI = "https://api.sunrise-sunset.org/json?tzid=europe/london&formatted=0&lat=" LATITUDE "&lng=" LONGITUDE;
uint16_t CSunrise::mSunriseMins, CSunrise::mSunsetMins;

void CSunrise::Update(void)
{
    Serial.println("Getting sunrise and sunset times");

    HTTPClient client;
    client.begin(mSunriseAPI);

    uint16_t respCode = client.GET();

    // Update the sunrise and sunset times if the API request was sucessful
    if (respCode == 200) {
        String payload = client.getString();

        // Get the sunrise and sunset times as char arrays from the response JSON.
        // These have format YYYY-MM-DDThh:mm:ss+00:00
        JsonDocument doc;
        deserializeJson(doc, payload);
        const char* sunrise = doc["results"]["sunrise"];
        const char* sunset = doc["results"]["sunset"];

        // Get the minutes since midnight by converting each char from ascii to integer, multiplying depending on position and adding.
        mSunriseMins = (sunrise[11] - '0') * 600 + (sunrise[12] - '0') * 60 + (sunrise[14] - '0') * 10 + (sunrise[15] - '0');
        mSunsetMins = (sunset[11] - '0') * 600 + (sunset[12] - '0') * 60 + (sunset[14] - '0') * 10 + (sunset[15] - '0');

        Serial.printf("New sunrise/sunset times: %d, %d\n", mSunriseMins, mSunsetMins);
    }
    else {
        Serial.printf("Could not updare sunrise and sunset times. Error code: %d\n", respCode);
    }
    // Free resources
    client.end();
}
