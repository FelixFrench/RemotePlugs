#include "CWebServer.hpp"
#include "CTransmitQueue.hpp"
#include "CRemoteCodes.hpp"

// Initialise static member variables
String CWebServer::header = String();
WiFiServer CWebServer::server = WiFiServer(80);
uint32_t CWebServer::connectionStartTime = 0;

const char CWebServer::ResponseHeader[] =   "HTTP/1.1 200 OK\n"
                                            "Content-type:text/html\n"
                                            "Connection: close\n";
                                            
const char CWebServer::HTML[] =     "<!DOCTYPE html><html><head>"
                                    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                                    "<link rel=\"icon\" href=\"data:,\">"

                                    // Style
                                    "<style>html { font-family: Helvetica; text-align: center; margin: auto;}"
                                    ".button { background-color: #555555; border: none; padding: 24px 50px;"
                                    "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
                                    "table{ margin-left: auto; margin-right: auto;}"
                                    "th { font-size: 30px; font-weight: bold;}"
                                    "td { font-size: 40px;}"
                                    "a { color: black; text-decoration: none;}"
                                    "</style></head>"

                                    // Main table
                                    "<body><table>"

                                        "<tr>"
                                          "<th><a href=\"/all_on\">On</a></th>"
                                          "<th></th>"
                                          "<th><a href=\"/all_off\">Off</a></th>"
                                        "</tr>"
                                    
                                        "<tr>"
                                            "<td><a href=\"/0_on\"><button class=\"button\"></button></a></td>"
                                            "<td>&#128719;</td>"
                                            "<td><a href=\"/0_off\"><button class=\"button\"></button></a></td>"
                                        "</tr>"

                                        "<tr>"
                                            "<td><a href=\"/1_on\"><button class=\"button\"></button></a></td>"
                                            "<td>?</td>"
                                            "<td><a href=\"/1_off\"><button class=\"button\"></button></a></td>"
                                            "</tr>"

                                        "<tr>"
                                            "<td><a href=\"/2_on\"><button class=\"button\"></button></a></td>"
                                            "<td>&#127969;</td>"
                                            "<td><a href=\"/2_off\"><button class=\"button\"></button></a></td>"
                                        "</tr>"

                                        "<tr>"
                                            "<td><a href=\"/3_on\"><button class=\"button\"></button></a></td>"
                                            "<td>&#127877;</td>"
                                            "<td><a href=\"/3_off\"><button class=\"button\"></button></a></td>"
                                        "</tr>"

                                        "<tr>"
                                            "<td><a href=\"/4_on\"><button class=\"button\"></button></a></td>"
                                            "<td>&#127876;</td>"
                                            "<td><a href=\"/4_off\"><button class=\"button\"></button></a></td>"
                                        "</tr>"

                                    "</table></body></html>";


void CWebServer::Background()
{
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

        // Send the response header
        client.println(ResponseHeader);
        
        // Send the webpage HTML
        client.println(HTML);
        
        // The response ends with a blank line
        client.println();

        // Handle any GET requests in the header requesting switching
        if (header.indexOf("GET /all_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_1_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_3_ON);
          CTransmitQueue::Push(CRemoteCodes::NEW_4_ON);
        } else if (header.indexOf("GET /all_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_1_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_3_OFF);
          CTransmitQueue::Push(CRemoteCodes::NEW_4_OFF);
        }
        else if (header.indexOf("GET /0_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_ON);
        } else if (header.indexOf("GET /0_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_0_OFF);
        }
        else if (header.indexOf("GET /1_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_1_ON);
        } else if (header.indexOf("GET /1_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_1_OFF);
        }
        else if (header.indexOf("GET /2_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_2_ON);
        } else if (header.indexOf("GET /2_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_2_OFF);
        }
        else if (header.indexOf("GET /3_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_3_ON);
        } else if (header.indexOf("GET /3_off") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_3_OFF);
        }
        else if (header.indexOf("GET /4_on") >= 0) {
          CTransmitQueue::Push(CRemoteCodes::NEW_4_ON);
        } else if (header.indexOf("GET /4_off") >= 0) {
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