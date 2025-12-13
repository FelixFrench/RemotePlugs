#include "CWebServer.hpp"
#include "CTransmitQueue.hpp"
#include "CRemoteCodes.hpp"
#include "LittleFS.h"

// Initialise static member variables
String CWebServer::header = String();
WiFiServer CWebServer::server = WiFiServer(80);
uint32_t CWebServer::connectionStartTime = 0;

// State variable
CWebServer::STATE CWebServer::mState;

void CWebServer::Background()
{

  if (mState == STATE_INITIAL) {
    if (LittleFS.begin(true)) {
      server.begin();
      mState = STATE_ACTIVE;
    }
    else {
      Serial.println("An error occurred while starting LittleFS");
    }
  }


  // Get any client that is connected and has data available for reading
  WiFiClient client = server.available();

  // If no client is available, do nothing.
  if(!client)
    return;

  // A client is available, record when it connected
  connectionStartTime = millis();

  Serial.println("Webserver client connected");

  // This will be cleared once something other than a \n or \r is received.
  bool currentLineIsEmpty = true;

  // While the client is connected and not timed out
  while (client.connected() && millis() - connectionStartTime <= timeoutTime) {

    // If there are no bytes to be read from the client right now, just keep looping
    if (!client.available())
      continue;

    // Read a a byte from the client
    char c = client.read();
    header += c;

    // If the byte is a newline
    if (c == '\n') {

      // If other bytes have been recieved on this line, just start a new line
      if (!currentLineIsEmpty) {
        currentLineIsEmpty = true;
      }
      // If \n was the first character on this line, then the client HTTP request is complete, so respond.
      else {
        Serial.print(header);

        // POST method - light action commands
        if (header.startsWith("POST /")) {

          // Just send a No Content header.
          client.println("HTTP/1.1 204 No Content");
          client.println("Connection: close");
        }

        // GET method - initial webpage load
        else if (header.startsWith("GET /")) {

          // TODO: Support for favicon.ico
          
          // Send an OK header saying html enclosed.
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          
          // Send the webpage HTML
          File htmlFile = LittleFS.open("/main.html");

          uint8_t fileBuf[512];
          while(htmlFile.available()){
            int x = 1;
            size_t bytesRead = htmlFile.read(fileBuf, sizeof(fileBuf));
            client.write(fileBuf, bytesRead);
          }
          htmlFile.close();
        }

        // Any other method not allowed
        else {
          client.println("HTTP/1.1 405 Method Not Allowed");
          client.println("Connection: close");
        }
        
        // The response ends with a blank line
        client.println();

        // Handle any GET requests in the header requesting switching
        if (header.indexOf("POST /all_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_1_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_3_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_4_ON);
        } else if (header.indexOf("POST /all_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_1_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_3_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_4_OFF);
        }
        else if (header.indexOf("POST /0_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_ON);
        } else if (header.indexOf("POST /0_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_OFF);
        }
        else if (header.indexOf("POST /1_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_1_ON);
        } else if (header.indexOf("POST /1_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_1_OFF);
        }
        else if (header.indexOf("POST /2_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
        } else if (header.indexOf("POST /2_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
        }
        else if (header.indexOf("POST /3_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_3_ON);
        } else if (header.indexOf("POST /3_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_3_OFF);
        }
        else if (header.indexOf("POST /4_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_4_ON);
        } else if (header.indexOf("POST /4_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_4_OFF);
        }

        // Break out of the while loop
        break;
      }
    }
    // If anything other than a carriage return character was recieved, the current line is now not empty.
    else if (c != '\r') {
      currentLineIsEmpty = false;
    }
  }

  // Clear the header variable
  header = "";

  // Close the connection
  client.stop();
  Serial.println("Client disconnected");
}