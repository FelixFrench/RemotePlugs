#include <Arduino.h>

#include "WiFi.h"
#include "time.h"
#include "CRemoteCodes.hpp"
#include "CTransmitter.hpp"
#include "CWebServer.hpp"
#include "NetCreds.h"

enum PIN {
  PIN_LED = 2,
};


const char* ntpServer = "pool.ntp.org";
// TimeZone rule for Europe/London including daylight adjustment rules.
// From https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
const char* time_zone = "GMT0BST,M3.5.0/1,M10.5.0";

struct tm timeinfo;

void setup() {

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Set up the transmitter
  CTransmitter::Setup();

  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(NetCreds::ssid);
  
  WiFi.begin(NetCreds::ssid, NetCreds::password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // Print own IP address
  Serial.println("Connection complete. IP address: ");
  Serial.println(WiFi.localIP());

  // Start web server
  CWebServer::Setup();

  configTzTime(time_zone, ntpServer);

  Serial.println("Getting time");
  while(!getLocalTime(&timeinfo));
  Serial.println("  complete");

  digitalWrite(PIN_LED, LOW);
}

void loop() {

  // If a request to transmit was received through the website, set the code to transmit and the number of repeats
  CRemoteCodes WebpageCode = CWebServer::Background();
  if (WebpageCode != CRemoteCodes::NONE){
    CTransmitter::StartTransmitting(WebpageCode);
  }
  
  // Get the time, and do some stuff once per second
  getLocalTime(&timeinfo);
  uint8_t nowSeconds = timeinfo.tm_sec;
  static uint8_t lastSeconds = 0;
  if(nowSeconds != lastSeconds){
    lastSeconds = nowSeconds;
    Serial.println(&timeinfo, "%a, %d %b %Y %H:%M:%S");
  }

  if (CTransmitter::Background())
    digitalWrite(PIN_LED, HIGH);
  else
    digitalWrite(PIN_LED, LOW);
}