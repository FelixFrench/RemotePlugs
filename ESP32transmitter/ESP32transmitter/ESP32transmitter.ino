#include "WiFi.h"
#include <RCSwitch.h>

enum PIN {

  PIN_LED = 2,
  PIN_TRANSMIT = 12
};

// The codes transmitted by the new remote.
enum NewCodeValues {
  NEW_CODE_0_ON = 15404668,
  NEW_CODE_0_OFF = 15404660,
  NEW_CODE_1_ON = 15404666,
  NEW_CODE_1_OFF = 15404658,
  NEW_CODE_2_ON = 15404665,
  NEW_CODE_2_OFF = 15404657,
  NEW_CODE_3_ON = 15404669,
  NEW_CODE_3_OFF = 15404661,
  NEW_CODE_4_ON = 15404667,
  NEW_CODE_4_OFF = 15404659,
};

// Set network credentials
const char* ssid = "peanutbutter";
const char* password = "01010101";

// Set web server port number to 80
WiFiServer server(80);

RCSwitch mySwitch = RCSwitch();

// The HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// The time at which handling of the current client's request began
unsigned long connectionStartTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 200;

void setup() {

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Set up the transmitter and define the pulse length and number of repititions
  mySwitch.enableTransmit(digitalPinToInterrupt(PIN_TRANSMIT));
  mySwitch.setPulseLength(150);
  mySwitch.setRepeatTransmit(100);

  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // Print own IP address and start web server
  Serial.println("Connection complete. IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  digitalWrite(PIN_LED, LOW);
}

void loop() {
  // Get any client that is connected and has data available for reading
  WiFiClient client = server.available();

  // If a client has connected and has data available
  if (client) {

    currentTime = millis();
    connectionStartTime = currentTime;

    Serial.println("Conn");
    digitalWrite(PIN_LED, HIGH);

    bool currentLineIsEmpty = true;

    // While the client is connected and not timed out
    while (client.connected() && currentTime - connectionStartTime <= timeoutTime) {
      currentTime = millis();

      // If there are bytes to be read from the client
      if (client.available()) {

        // Read a a byte from the client and print it out
        char c = client.read();
        //Serial.write(c);
        header += c;

        // If the byte is a newline
        if (c == '\n') {

          // If \n was the first character on this line, then the client HTTP request is complete, so respond.
          if (currentLineIsEmpty) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Handle any GET requests in the header requesting switching
            if (header.indexOf("GET /0_on") >= 0) {
              Serial.println("0 on");
              mySwitch.send(NEW_CODE_0_ON, 24);
            } else if (header.indexOf("GET /0_off") >= 0) {
              Serial.println("0 off");
              mySwitch.send(NEW_CODE_0_OFF, 24);

            } else if (header.indexOf("GET /1_on") >= 0) {
              Serial.println("1 on");
              mySwitch.send(NEW_CODE_1_ON, 24);
            } else if (header.indexOf("GET /1_off") >= 0) {
              Serial.println("1 off");
              mySwitch.send(NEW_CODE_1_OFF, 24);

            } else if (header.indexOf("GET /2_on") >= 0) {
              Serial.println("2 on");
              mySwitch.send(NEW_CODE_2_ON, 24);
            } else if (header.indexOf("GET /2_off") >= 0) {
              Serial.println("2 off");
              mySwitch.send(NEW_CODE_2_OFF, 24);

            } else if (header.indexOf("GET /3_on") >= 0) {
              Serial.println("3 on");
              mySwitch.send(NEW_CODE_3_ON, 24);
            } else if (header.indexOf("GET /3_off") >= 0) {
              Serial.println("3 off");
              mySwitch.send(NEW_CODE_3_OFF, 24);

            } else if (header.indexOf("GET /4_on") >= 0) {
              Serial.println("4 on");
              mySwitch.send(NEW_CODE_4_ON, 24);
            } else if (header.indexOf("GET /4_off") >= 0) {
              Serial.println("4 off");
              mySwitch.send(NEW_CODE_4_OFF, 24);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html><head>");
            client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // Style
            client.println("<style>html { font-family: Helvetica; text-align: center; margin: auto;}");
            client.println(".button { background-color: #555555; border: none; padding: 24px 50px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("table { margin-left: auto; margin-right: auto;}");
            client.println("</style></head>");

            // Main table
            client.println("<table><tr><th>On</th> <th></th> <th>Off</th></tr>");

            client.println("<tr>");
            client.println("<td><a href=\"/0_on\"><button class=\"button\"></button></a></td>");
            client.println("<td>0</td>");
            client.println("<td><a href=\"/0_off\"><button class=\"button\"></button></a></td>");
            client.println("</tr>");

            client.println("<tr>");
            client.println("<td><a href=\"/1_on\"><button class=\"button\"></button></a></td>");
            client.println("<td>1</td>");
            client.println("<td><a href=\"/1_off\"><button class=\"button\"></button></a></td>");
            client.println("</tr>");

            client.println("<tr>");
            client.println("<td><a href=\"/2_on\"><button class=\"button\"></button></a></td>");
            client.println("<td>2</td>");
            client.println("<td><a href=\"/2_off\"><button class=\"button\"></button></a></td>");
            client.println("</tr>");

            client.println("<tr>");
            client.println("<td><a href=\"/3_on\"><button class=\"button\"></button></a></td>");
            client.println("<td>3</td>");
            client.println("<td><a href=\"/3_off\"><button class=\"button\"></button></a></td>");
            client.println("</tr>");

            client.println("<tr>");
            client.println("<td><a href=\"/4_on\"><button class=\"button\"></button></a></td>");
            client.println("<td>4</td>");
            client.println("<td><a href=\"/4_off\"><button class=\"button\"></button></a></td>");
            client.println("</tr>");

            client.println("</table></body></html>");

            // The HTTP response ends with a blank line
            client.println();
            // Break out of the while loop
            break;

          // If a newline character is received, clear the current line
          } else {
            currentLineIsEmpty = true;
          }

        // If anything other than a carriage return character was recieved, add it to the end of the current line
        } else if (c != '\r') {
          currentLineIsEmpty = false;
        }
      }
    }

    Serial.println(header);
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Disconn");
    digitalWrite(PIN_LED, LOW);
  }
}
