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
      if (path.startsWith("/?command=")) {
        // Extract the command
        String commandEncoded = path.substring(10); // After "/?command="
        String command = urlDecode(commandEncoded);
        Serial.print("Received Command: ");
        Serial.println(command);

        // Process the command
        processCommand(command);
      } else if (path.startsWith("/?")) {
        // Handle button commands
        processButtonCommand(path);
      }
    }

    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    // HTML content with command input and buttons
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<title>Arduino Servo and Electromagnet Control</title>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
    client.println(".control-panel { margin: 20px 0; padding: 10px; background-color: #f0f0f0; border-radius: 5px; }");
    client.println("button { padding: 10px 20px; margin: 5px; background-color: #00BCD4; color: white; border: none; border-radius: 4px; cursor: pointer; }");
    client.println("button:hover { background-color: #0097A7; }");
    client.println("input[type=text] { padding: 10px; width: 80%; margin: 5px; border: 1px solid #ccc; border-radius: 4px; }");
    client.println("</style>");
    client.println("<script>");
    // JavaScript to send command on Enter key
    client.println("document.addEventListener('DOMContentLoaded', function() {");
    client.println("  var input = document.getElementById('commandInput');");
    client.println("  input.addEventListener('keyup', function(event) {");
    client.println("    if (event.key === 'Enter') {");
    client.println("      event.preventDefault();");
    client.println("      sendCommand();");
    client.println("    }");
    client.println("  });");
    client.println("});");
    client.println("function sendCommand() {");
    client.println("  var command = document.getElementById('commandInput').value;");
    client.println("  var xhr = new XMLHttpRequest();");
    client.println("  xhr.open('GET', '/?command=' + encodeURIComponent(command), true);");
    client.println("  xhr.send();");
    client.println("  document.getElementById('commandInput').value = '';"); // Clear the input
    client.println("}");
    client.println("</script>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>Arduino Servo and Electromagnet Control Panel</h1>");

    // Command Input Section
    client.println("<div class='control-panel'>");
    client.println("<h2>Enter Command</h2>");
    client.println("<input type='text' id='commandInput' placeholder='Type your command here'>");
    client.println("<button onclick='sendCommand()'>Send</button>");
    client.println("<p>Example commands:</p>");
    client.println("<ul>");
    client.println("<li><code>servo9 open</code></li>");
    client.println("<li><code>servo10 middle</code></li>");
    client.println("<li><code>servo11 close</code></li>");
    client.println("<li><code>magnet1 on</code></li>");
    client.println("<li><code>magnet2 off</code></li>");
    client.println("</ul>");
    client.println("</div>");

    // Current Status
    client.println("<div class='control-panel'>");
    client.println("<h2>Current Status</h2>");
    client.println("<p><strong>Servo 9 Position:</strong> " + String(pos9) + "&deg;</p>");
    client.println("<p><strong>Servo 10 Position:</strong> " + String(pos10) + "&deg;</p>");
    client.println("<p><strong>Servo 11 Position:</strong> " + String(pos11) + "&deg;</p>");
    client.println("<p><strong>Electromagnet 1 State:</strong> " + String(pin13State == HIGH ? "On" : "Off") + "</p>");
    client.println("<p><strong>Electromagnet 2 State:</strong> " + String(pin8State == HIGH ? "On" : "Off") + "</p>");
    client.println("</div>");

    // Button Controls Section
    client.println("<div class='control-panel'>");
    client.println("<h2>Button Controls</h2>");
    // Servo 9 buttons
    client.println("<h3>Servo 9</h3>");
    client.println("<button onclick=\"location.href='/?servo=9&dir=close'\">Close (0&deg;)</button>");
    client.println("<button onclick=\"location.href='/?servo=9&dir=middle'\">Middle (90&deg;)</button>");
    client.println("<button onclick=\"location.href='/?servo=9&dir=open'\">Open (180&deg;)</button>");
    // Servo 10 buttons
    client.println("<h3>Servo 10</h3>");
    client.println("<button onclick=\"location.href='/?servo=10&dir=close'\">Close (0&deg;)</button>");
    client.println("<button onclick=\"location.href='/?servo=10&dir=middle'\">Middle (90&deg;)</button>");
    client.println("<button onclick=\"location.href='/?servo=10&dir=open'\">Open (180&deg;)</button>");
    // Servo 11 buttons
    client.println("<h3>Servo 11</h3>");
    client.println("<button onclick=\"location.href='/?servo=11&dir=close'\">Close (0&deg;)</button>");
    client.println("<button onclick=\"location.href='/?servo=11&dir=middle'\">Middle (90&deg;)</button>");
    client.println("<button onclick=\"location.href='/?servo=11&dir=open'\">Open (180&deg;)</button>");
    // Electromagnet 1 buttons
    client.println("<h3>Electromagnet 1</h3>");
    client.println("<button onclick=\"location.href='/?magnet=1&state=on'\">Turn On</button>");
    client.println("<button onclick=\"location.href='/?magnet=1&state=off'\">Turn Off</button>");
    // Electromagnet 2 buttons
    client.println("<h3>Electromagnet 2</h3>");
    client.println("<button onclick=\"location.href='/?magnet=2&state=on'\">Turn On</button>");
    client.println("<button onclick=\"location.href='/?magnet=2&state=off'\">Turn Off</button>");
    // All Servos Control
    client.println("<h3>All Servos</h3>");
    client.println("<button onclick=\"location.href='/?all=open'\">Open All Servos (180&deg;)</button>");
    client.println("<button onclick=\"location.href='/?all=close'\">Close All Servos (0&deg;)</button>");
    client.println("</div>");

    client.println("</body></html>");

    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}

// Function to process commands
void processCommand(String command) {
  command.trim(); // Remove leading and trailing whitespace
  command.toLowerCase(); // Convert to lowercase for case-insensitive comparison

  if (command.startsWith("servo")) {
    // Control servos
    // Find the space after 'servoX'
    int spaceIndex = command.indexOf(' ', 5);
    if (spaceIndex == -1) {
      Serial.println("Invalid command format for servo.");
      return;
    }

    String servoNumStr = command.substring(5, spaceIndex); // Extract servo number string
    servoNumStr.trim();
    int servoNum = servoNumStr.toInt();
    String action = command.substring(spaceIndex + 1);
    action.trim();

    Serial.print("Servo Command: servo");
    Serial.print(servoNum);
    Serial.print(" ");
    Serial.println(action);

    int* currentPos = nullptr;
    Servo* targetServo = nullptr;

    if (servoNum == 9) {
      currentPos = &pos9;
      targetServo = &servo9;
    } else if (servoNum == 10) {
      currentPos = &pos10;
      targetServo = &servo10;
    } else if (servoNum == 11) {
      currentPos = &pos11;
      targetServo = &servo11;
    } else {
      Serial.println("Invalid servo number.");
      return;
    }

    if (action == "close") {
      *currentPos = MIN_ANGLE;
    } else if (action == "open") {
      *currentPos = MAX_ANGLE;
    } else if (action == "middle") {
      *currentPos = CENTER_ANGLE;
    } else {
      Serial.println("Invalid action for servo.");
      return;
    }

    targetServo->write(*currentPos);
    Serial.print("Moved servo ");
    Serial.print(servoNum);
    Serial.print(" to position ");
    Serial.println(*currentPos);
  } else if (command.startsWith("magnet")) {
    // Control electromagnets
    // Find the space after 'magnetX'
    int spaceIndex = command.indexOf(' ', 6);
    if (spaceIndex == -1) {
      Serial.println("Invalid command format for magnet.");
      return;
    }

    String magnetNumStr = command.substring(6, spaceIndex); // Extract magnet number string
    magnetNumStr.trim();
    int magnetNum = magnetNumStr.toInt();
    String action = command.substring(spaceIndex + 1);
    action.trim();

    Serial.print("Magnet Command: magnet");
    Serial.print(magnetNum);
    Serial.print(" ");
    Serial.println(action);

    int* magnetState = nullptr;
    int pin = 0;

    if (magnetNum == 1) {
      magnetState = &pin13State;
      pin = 13;
    } else if (magnetNum == 2) {
      magnetState = &pin8State;
      pin = 8;
    } else {
      Serial.println("Invalid magnet number.");
      return;
    }

    if (action == "on") {
      *magnetState = HIGH;
    } else if (action == "off") {
      *magnetState = LOW;
    } else {
      Serial.println("Invalid action for magnet.");
      return;
    }

    digitalWrite(pin, *magnetState);
    Serial.print("Set Electromagnet ");
    Serial.print(magnetNum);
    Serial.print(" (pin ");
    Serial.print(pin);
    Serial.print(") to ");
    Serial.println(*magnetState == HIGH ? "HIGH" : "LOW");
  } else {
    Serial.println("Unknown command.");
  }
}

// Function to process button commands
void processButtonCommand(String path) {
  if (path.startsWith("/?servo=")) {
    int servoIndex = path.indexOf("servo=");
    int dirIndex = path.indexOf("dir=");
    if (servoIndex != -1 && dirIndex != -1) {
      String servoNumStr = path.substring(servoIndex + 6, path.indexOf('&', servoIndex));
      int servoNum = servoNumStr.toInt();
      String dirStr = path.substring(dirIndex + 4);
      dirStr = dirStr.substring(0, dirStr.indexOf('&') == -1 ? dirStr.length() : dirStr.indexOf('&'));
      String command = "servo" + String(servoNum) + " " + dirStr;
      processCommand(command);
    }
  } else if (path.startsWith("/?magnet=")) {
    int magnetIndex = path.indexOf("magnet=");
    int stateIndex = path.indexOf("state=");
    if (magnetIndex != -1 && stateIndex != -1) {
      String magnetNumStr = path.substring(magnetIndex + 7, path.indexOf('&', magnetIndex));
      int magnetNum = magnetNumStr.toInt();
      String stateStr = path.substring(stateIndex + 6);
      stateStr = stateStr.substring(0, stateStr.indexOf('&') == -1 ? stateStr.length() : stateStr.indexOf('&'));
      String command = "magnet" + String(magnetNum) + " " + stateStr;
      processCommand(command);
    }
  } else if (path.startsWith("/?all=")) {
    String action = path.substring(6);
    if (action.startsWith("open")) {
      moveAllServos(MAX_ANGLE);
    } else if (action.startsWith("close")) {
      moveAllServos(MIN_ANGLE);
    }
  }
}

// Function to URL-decode a percent-encoded string
String urlDecode(String input) {
  String decoded = "";
  char c;
  for (unsigned int i = 0; i < input.length(); i++) {
    c = input.charAt(i);
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < input.length()) {
      String hex = input.substring(i + 1, i + 3);
      char decodedChar = (char) strtol(hex.c_str(), NULL, 16);
      decoded += decodedChar;
      i += 2;
    } else {
      decoded += c;
    }
  }
  return decoded;
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
