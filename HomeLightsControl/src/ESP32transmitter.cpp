#include <Arduino.h>

#include "WiFi.h"
#include "CTransmitter.hpp"
#include "CWebServer.hpp"
#include "CTimedActions.hpp"
#include "NetCreds.h"

enum PIN {
  PIN_LED = 2,
};


void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Set up the transmitter
  CTransmitter::Setup();

  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting WiFi ");  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // Print own IP address
  Serial.println(WiFi.localIP());

  // Start web server
  CWebServer::Setup();

  digitalWrite(PIN_LED, LOW);
}

void loop()
{
  // Check if any transmissions have been requested from the webpage. Any which have will be added to the queue.
  CWebServer::Background();

  // Check if transmissions need to be sent at the current time. Any which do will be added to the queue.
  CTimedActions::Run();

  // Run the transmitter backround task. This will return true if it sent a transmission.
  if (CTransmitter::Background())
    digitalWrite(PIN_LED, HIGH);
  else
    digitalWrite(PIN_LED, LOW);
}