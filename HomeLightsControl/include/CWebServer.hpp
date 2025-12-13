#ifndef C_WEB_SERVER_HPP
#define C_WEB_SERVER_HPP

#include "stdlib.h"
#include "WiFi.h"

class CWebServer {

public:

// Set up the webserver. Must be called before use.
static void Setup(void);

// Send a transmission if required. Returns true if a transmission was made.
static void Background(void);

private:
// Set web server port number to 80
static WiFiServer server;

// The HTTP request
static String header;

// The time at which handling of the current client's request began
static uint32_t connectionStartTime;

// Define timeout time in milliseconds (example: 2000ms = 2s)
static const uint32_t timeoutTime = 200;

// The HTTP header of the server's response.
static const char ResponseHeader[];

};
#endif