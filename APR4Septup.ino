#include <WiFiS3.h>
#include <Servo.h>

// Set the SSID and Password for the access point
char ssid[] = "RVR_Network";    // Access point name
char pass[] = "123456789";      // Password for WPA2
WiFiServer server(80);          // Create a server on port 80

// Create servo objects
Servo servo9;
Servo servo10;
Servo servo11;
// Pin 13 and pin 8 are used as digital outputs
// Pin 13 controls Electromagnet 1
// Pin 8 controls Electromagnet 2

// Current positions of servos
int pos9 = 90;
int pos10 = 90;
int pos11 = 90;

// Define angle constants
const int MIN_ANGLE = 0;
const int CENTER_ANGLE = 90;
const int MAX_ANGLE = 180;

// States of pin 13 and pin 8
int pin13State = LOW; // Start with Electromagnet 1 off
int pin8State = LOW;  // Start with Electromagnet 2 off

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // Attach servos to pins
  servo9.attach(9);
  servo10.attach(10);
  servo11.attach(11);

  // Set initial positions
  servo9.write(pos9);
  servo10.write(pos10);
  servo11.write(pos11);

  // Set up pin 13 as output for Electromagnet 1
  pinMode(13, OUTPUT);
  digitalWrite(13, pin13State);

  // Set up pin 8 as output for Electromagnet 2
  pinMode(8, OUTPUT);
  digitalWrite(8, pin8State);

  // Check if the WiFi module is present
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  // Attempt to start the access point
  Serial.print("Attempting to start AP with SSID: ");
  Serial.println(ssid);

  // Start the AP on channel 1
  if (WiFi.beginAP(ssid, pass, 1)) {
    Serial.println("Access Point started successfully!");
  } else {
    Serial.println("Failed to start Access Point.");
    while (true);
  }

  // Start the server
  server.begin();
  Serial.println("Server started.");

  // Print the IP address of the AP
  IPAddress myIP = WiFi.localIP();
  Serial.print("AP IP Address: ");
  Serial.println(myIP);
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected.");
    String requestLine = "";

    // Read the request line
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          // End of the request line
          break;
        } else if (c != '\r') {
          requestLine += c;
        }
      }
    }

    Serial.print("Request Line: ");
    Serial.println(requestLine);  // Optional debug print

    // Process the request line
    if (requestLine.startsWith("GET ")) {
      int pathStart = 4; // After "GET "
      int pathEnd = requestLine.indexOf(' ', pathStart);
      String path = requestLine.substring(pathStart, pathEnd);

      Serial.print("Path: ");
      Serial.println(path);  // Optional debug print

      // Process commands based on the path
      if (path.startsWith("/?all=close")) {
        // Move all servos to MIN_ANGLE
        moveAllServos(MIN_ANGLE);
        Serial.println("All servos moved to close position.");
      } else if (path.startsWith("/?all=open")) {
        // Move all servos to MAX_ANGLE
        moveAllServos(MAX_ANGLE);
        Serial.println("All servos moved to open position.");
      } else if (path.indexOf("/?servo=") != -1) {
        int servoIndex = path.indexOf("servo=");
        int dirIndex = path.indexOf("dir=");

        if (servoIndex != -1 && dirIndex != -1) {
          // Extract servo number
          int servoNumEnd = path.indexOf('&', servoIndex);
          if (servoNumEnd == -1) servoNumEnd = path.length();
          String servoStr = path.substring(servoIndex + 6, servoNumEnd);
          int servoNum = servoStr.toInt();

          // Extract direction
          int dirEnd = path.indexOf('&', dirIndex);
          if (dirEnd == -1) dirEnd = path.length();
          String direction = path.substring(dirIndex + 4, dirEnd);

          Serial.print("Servo: ");
          Serial.print(servoNum);
          Serial.print(" Direction: ");
          Serial.println(direction);

          // Process servo movement
          int* currentPos = nullptr;
          Servo* targetServo = nullptr;

          switch(servoNum) {
            case 9:
              currentPos = &pos9;
              targetServo = &servo9;
              break;
            case 10:
              currentPos = &pos10;
              targetServo = &servo10;
              break;
            case 11:
              currentPos = &pos11;
              targetServo = &servo11;
              break;
            default:
              break;
          }

          if (currentPos != nullptr && targetServo != nullptr) {
            if (direction == "close") {
              *currentPos = MIN_ANGLE;
            } else if (direction == "open") {
              *currentPos = MAX_ANGLE;
            } else if (direction == "middle") {
              *currentPos = CENTER_ANGLE;
            }

            targetServo->write(*currentPos);
            Serial.print("Moving servo ");
            Serial.print(servoNum);
            Serial.print(" to position: ");
            Serial.println(*currentPos);
          }
        }
      } else if (path.indexOf("/?pin13=") != -1) {
        // Control Electromagnet 1 (pin 13) on/off
        int pin13Index = path.indexOf("pin13=");
        if (pin13Index != -1) {
          int stateEnd = path.indexOf('&', pin13Index);
          if (stateEnd == -1) stateEnd = path.length();
          String state = path.substring(pin13Index + 6, stateEnd);

          Serial.print("Electromagnet 1 State: ");
          Serial.println(state);

          if (state == "on") {
            pin13State = HIGH;
          } else if (state == "off") {
            pin13State = LOW;
          }

          digitalWrite(13, pin13State);
          Serial.print("Set Electromagnet 1 (pin 13) to: ");
          Serial.println(pin13State == HIGH ? "HIGH" : "LOW");
        }
      } else if (path.indexOf("/?magnet=") != -1) {
        // Control Electromagnet 2 (pin 8) on/off
        int magnetIndex = path.indexOf("magnet=");
        if (magnetIndex != -1) {
          int stateEnd = path.indexOf('&', magnetIndex);
          if (stateEnd == -1) stateEnd = path.length();
          String state = path.substring(magnetIndex + 7, stateEnd);

          Serial.print("Electromagnet 2 State: ");
          Serial.println(state);

          if (state == "on") {
            pin8State = HIGH;
          } else if (state == "off") {
            pin8State = LOW;
          }

          digitalWrite(8, pin8State);
          Serial.print("Set Electromagnet 2 (pin 8) to: ");
          Serial.println(pin8State == HIGH ? "HIGH" : "LOW");
        }
      }
    }

    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    // HTML content
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<title>Arduino Servo Control</title>");
    client.println("<style>");
    // Updated button styles to make them cyan
    client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
    client.println(".servo-control { margin: 20px 0; padding: 10px; background-color: #f0f0f0; border-radius: 5px; }");
    client.println("button { padding: 10px 20px; margin: 5px; background-color: #00BCD4; color: white; border: none; border-radius: 4px; cursor: pointer; }");
    client.println("button:hover { background-color: #0097A7; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>Arduino Servo Control Panel</h1>");

    // All Servos Control
    client.println("<div class='servo-control'>");
    client.println("<h2>All Servos Control</h2>");
    client.println("<button onclick=\"window.location.href='/?all=open'\">Open All Servos (180&deg;)</button>");
    client.println("<button onclick=\"window.location.href='/?all=close'\">Close All Servos (0&deg;)</button>");
    client.println("</div>");

    // Servo 9 controls
    client.println("<div class='servo-control'>");
    client.println("<h2>Servo on Pin 9</h2>");
    client.println("<button onclick=\"window.location.href='/?servo=9&dir=close'\">Close (0&deg;)</button>");
    client.println("<button onclick=\"window.location.href='/?servo=9&dir=middle'\">Middle (90&deg;)</button>");
    client.println("<button onclick=\"window.location.href='/?servo=9&dir=open'\">Open (180&deg;)</button>");
    client.println("<span>Current Position: " + String(pos9) + "&deg;</span>");
    client.println("</div>");

    // Servo 10 controls
    client.println("<div class='servo-control'>");
    client.println("<h2>Servo on Pin 10</h2>");
    client.println("<button onclick=\"window.location.href='/?servo=10&dir=close'\">Close (0&deg;)</button>");
    client.println("<button onclick=\"window.location.href='/?servo=10&dir=middle'\">Middle (90&deg;)</button>");
    client.println("<button onclick=\"window.location.href='/?servo=10&dir=open'\">Open (180&deg;)</button>");
    client.println("<span>Current Position: " + String(pos10) + "&deg;</span>");
    client.println("</div>");

    // Servo 11 controls
    client.println("<div class='servo-control'>");
    client.println("<h2>Servo on Pin 11</h2>");
    client.println("<button onclick=\"window.location.href='/?servo=11&dir=close'\">Close (0&deg;)</button>");
    client.println("<button onclick=\"window.location.href='/?servo=11&dir=middle'\">Middle (90&deg;)</button>");
    client.println("<button onclick=\"window.location.href='/?servo=11&dir=open'\">Open (180&deg;)</button>");
    client.println("<span>Current Position: " + String(pos11) + "&deg;</span>");
    client.println("</div>");

    // Electromagnet 1 (Pin 13) controls
    client.println("<div class='servo-control'>");
    client.println("<h2>Electromagnet 1 (Pin 13)</h2>");
    client.println("<button onclick=\"window.location.href='/?pin13=on'\">Turn On</button>");
    client.println("<button onclick=\"window.location.href='/?pin13=off'\">Turn Off</button>");
    client.println("<span>Current State: " + String(pin13State == HIGH ? "On" : "Off") + "</span>");
    client.println("</div>");

    // Electromagnet 2 (Pin 8) controls
    client.println("<div class='servo-control'>");
    client.println("<h2>Electromagnet 2 (Pin 8)</h2>");
    client.println("<button onclick=\"window.location.href='/?magnet=on'\">Turn On</button>");
    client.println("<button onclick=\"window.location.href='/?magnet=off'\">Turn Off</button>");
    client.println("<span>Current State: " + String(pin8State == HIGH ? "On" : "Off") + "</span>");
    client.println("</div>");

    client.println("</body></html>");

    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}

// Function to move all servos to a specific position
void moveAllServos(int angle) {
  // Move all servos
  servo9.write(angle);
  servo10.write(angle);
  servo11.write(angle);

  // Update current positions
  pos9 = angle;
  pos10 = angle;
  pos11 = angle;

  Serial.print("All servos moved to position: ");
  Serial.println(angle);
}
