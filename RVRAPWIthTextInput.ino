#include <WiFiS3.h>
#include <Servo.h>

// Set the SSID and Password for the access point
char ssid[] = "RVR_Network";    // Access point name
char pass[] = "123456789";      // Password for WPA2
WiFiServer server(80);          // Create a server on port 80

// Create servo objects
Servo servo9;
Servo servo10; // Axon Mini
Servo servo11; // Axon Mini

// Current positions of servos
int pos9 = 90;

// Define angle constants
const int MAX_ANGLE = 180;
const int MIN_ANGLE = 0;

// Define angle constants for Servo 10 (Axon Mini)
const int SERVO10_DOWN_SPEED = 170; // Adjust as needed
const int SERVO10_UP_SPEED = 10;    // Adjust as needed
const int SERVO10_STOP_ANGLE = 90;  // Adjust as needed

// Define angle constants for Servo 11 (Axon Mini)
const int SERVO11_DOWN_SPEED = 170; // Adjust as needed
const int SERVO11_UP_SPEED = 10;    // Adjust as needed
const int SERVO11_STOP_ANGLE = 90;  // Adjust as needed

// States of pin 13 and pin 8
int pin13State = LOW; // Electromagnet 1 (Pin 13)
int pin8State = LOW;  // Electromagnet 2 (Pin 8)

// Variables for Servo 10 movement
bool servo10Moving = false;
unsigned long servo10MoveStartTime = 0;
unsigned long servo10MoveDuration = 3000; // Default duration in ms
int servo10Direction = 0; // 1 for down, -1 for up

// Variables for Servo 11 movement
bool servo11Moving = false;
unsigned long servo11MoveStartTime = 0;
unsigned long servo11MoveDuration = 3000; // Default duration in ms
int servo11Direction = 0; // 1 for down, -1 for up

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
  servo10.write(SERVO10_STOP_ANGLE); // Stop position
  servo11.write(SERVO11_STOP_ANGLE); // Stop position

  // Set up pins as outputs for electromagnets
  pinMode(13, OUTPUT);
  digitalWrite(13, pin13State);

  pinMode(8, OUTPUT);
  digitalWrite(8, pin8State);

  // Check if the WiFi module is present
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  // Start the access point
  Serial.print("Starting AP with SSID: ");
  Serial.println(ssid);

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
  // Handle Servo 10 movement timing
  if (servo10Moving) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - servo10MoveStartTime;
    if (elapsedTime >= servo10MoveDuration) {
      // Stop Servo 10 after specified duration
      servo10.write(SERVO10_STOP_ANGLE); // Stop position
      servo10Moving = false;
      Serial.println("Servo 10 movement completed.");
    }
  }

  // Handle Servo 11 movement timing
  if (servo11Moving) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - servo11MoveStartTime;
    if (elapsedTime >= servo11MoveDuration) {
      // Stop Servo 11 after specified duration
      servo11.write(SERVO11_STOP_ANGLE); // Stop position
      servo11Moving = false;
      Serial.println("Servo 11 movement completed.");
    }
  }

  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected.");
    String requestLine = "";

    // Read the request line
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        requestLine += c;
        if (c == '\n') {
          // End of the request line
          break;
        }
      }
    }

    Serial.print("Request Line: ");
    Serial.println(requestLine);

    // Process the request line
    if (requestLine.startsWith("GET ")) {
      int pathStart = 4; // After "GET "
      int pathEnd = requestLine.indexOf(' ', pathStart);
      String path = requestLine.substring(pathStart, pathEnd);

      Serial.print("Path: ");
      Serial.println(path);

      // Process commands based on the path
      if (path.startsWith("/?command=")) {
        // Extract the command
        String commandEncoded = path.substring(10); // After "/?command="
        String command = urlDecode(commandEncoded);
        Serial.print("Received Command: ");
        Serial.println(command);

        // Process the command
        processCommand(command);

        // Send HTTP response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("OK");
      }
      else if (path.startsWith("/status")) {
        // Handle status request
        sendStatus(client);
      }
      else {
        // Serve the main page
        servePage(client);
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}

// Function to serve the main page
void servePage(WiFiClient& client) {
  // Send HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  // HTML content with command input and buttons
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Arduino Control Panel</title>");
  client.println("<style>");
  // CSS styles
  client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
  client.println(".control-panel { margin: 20px 0; }");
  client.println(".panel { background-color: #f0f0f0; border-radius: 5px; padding: 10px; box-sizing: border-box; }");
  client.println(".panel-half { width: 48%; display: inline-block; vertical-align: top; margin-right: 2%; }");
  client.println(".panel-half:last-child { margin-right: 0; }");
  client.println("button { padding: 10px 20px; margin: 5px; background-color: #00BCD4; color: white; border: none; border-radius: 4px; cursor: pointer; }");
  client.println("button:hover { background-color: #0097A7; }");
  client.println("input[type=text] { padding: 10px; width: calc(100% - 22px); margin: 5px 0; border: 1px solid #ccc; border-radius: 4px; }");
  client.println("</style>");
  client.println("<script>");
  // JavaScript to send command on Enter key and update status
  client.println("document.addEventListener('DOMContentLoaded', function() {");
  client.println("  var input = document.getElementById('commandInput');");
  client.println("  input.addEventListener('keyup', function(event) {");
  client.println("    if (event.key === 'Enter') {");
  client.println("      event.preventDefault();");
  client.println("      sendCommand();");
  client.println("    }");
  client.println("  });");
  client.println("  // Start status updates");
  client.println("  setInterval(updateStatus, 1000);");
  client.println("});");
  client.println("function sendCommand() {");
  client.println("  var command = document.getElementById('commandInput').value;");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.open('GET', '/?command=' + encodeURIComponent(command), true);");
  client.println("  xhr.send();");
  client.println("  document.getElementById('commandInput').value = '';"); // Clear the input
  client.println("}");
  client.println("function sendCommandFromButton(cmd) {");
  client.println("  document.getElementById('commandInput').value = cmd;");
  client.println("  sendCommand();");
  client.println("}");
  client.println("function updateStatus() {");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.open('GET', '/status', true);");
  client.println("  xhr.onreadystatechange = function() {");
  client.println("    if (xhr.readyState == 4 && xhr.status == 200) {");
  client.println("      var status = JSON.parse(xhr.responseText);");
  client.println("      document.getElementById('servo9Status').innerHTML = status.pos9 + '&deg;';");
  client.println("      document.getElementById('servo10Status').innerHTML = status.servo10State;");
  client.println("      document.getElementById('servo11Status').innerHTML = status.servo11State;");
  client.println("      document.getElementById('electro1Status').innerHTML = status.electro1;");
  client.println("      document.getElementById('electro2Status').innerHTML = status.electro2;");
  client.println("    }");
  client.println("  };");
  client.println("  xhr.send();");
  client.println("}");
  client.println("</script>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>Arduino Control Panel</h1>");

  // Command Input Section
  client.println("<div class='control-panel'>");
  client.println("<div class='panel'>");
  client.println("<h2>Enter Single-Key Command</h2>");
  client.println("<input type='text' id='commandInput' maxlength='1' placeholder='Type a single key command'>");
  client.println("<button onclick='sendCommand()'>Send</button>");
  client.println("<p>Single-Key Commands:</p>");
  client.println("<ul>");
  client.println("<li><strong>Servo 9:</strong> 'q' (0&deg;), 'w' (90&deg;), 'e' (180&deg;)</li>");
  client.println("<li><strong>Servo 10 (Axon Mini):</strong>");
  client.println("'a' (Down 3s), 'b' (Down 1s), 'c' (Down 0.5s), 'd' (Up 3s), 'n' (Up 1s), 'f' (Up 0.5s)</li>");
  client.println("<li><strong>Servo 11 (Axon Mini):</strong>");
  client.println("'g' (Down 3s), 'h' (Down 1s), 'i' (Down 0.5s), 'j' (Up 3s), 'k' (Up 1s), 'l' (Up 0.5s)</li>");
  client.println("<li><strong>All Servos:</strong> 'o' (Open All), 'm' (Middle All), 'p' (Close All)</li>");
  client.println("<li><strong>Electromagnet 1:</strong> '1' (On), '2' (Off)</li>");
  client.println("<li><strong>Electromagnet 2:</strong> '3' (On), '4' (Off)</li>");
  client.println("</ul>");
  client.println("</div>");
  client.println("</div>");

  // Begin side-by-side section
  client.println("<div class='control-panel'>");

  // Button Controls Section (Left Side)
  client.println("<div class='panel panel-half'>");
  client.println("<h2>Button Controls</h2>");
  // Servo 9 buttons
  client.println("<h3>Servo 9</h3>");
  client.println("<button onclick=\"sendCommandFromButton('q')\">Position 0&deg;</button>");
  client.println("<button onclick=\"sendCommandFromButton('w')\">Position 90&deg;</button>");
  client.println("<button onclick=\"sendCommandFromButton('e')\">Position 180&deg;</button>");
  // Servo 10 buttons (Axon Mini)
  client.println("<h3>Servo 10 (Axon Mini)</h3>");
  client.println("<p>Down:</p>");
  client.println("<button onclick=\"sendCommandFromButton('a')\">Down (3s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('b')\">Down (1s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('c')\">Down (0.5s)</button>");
  client.println("<p>Up:</p>");
  client.println("<button onclick=\"sendCommandFromButton('d')\">Up (3s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('n')\">Up (1s)</button>"); // Updated command
  client.println("<button onclick=\"sendCommandFromButton('f')\">Up (0.5s)</button>");
  // Servo 11 buttons (Axon Mini)
  client.println("<h3>Servo 11 (Axon Mini)</h3>");
  client.println("<p>Down:</p>");
  client.println("<button onclick=\"sendCommandFromButton('g')\">Down (3s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('h')\">Down (1s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('i')\">Down (0.5s)</button>");
  client.println("<p>Up:</p>");
  client.println("<button onclick=\"sendCommandFromButton('j')\">Up (3s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('k')\">Up (1s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('l')\">Up (0.5s)</button>");
  // All Servos Control
  client.println("<h3>All Servos</h3>");
  client.println("<button onclick=\"sendCommandFromButton('o')\">Open All Servos</button>");
  client.println("<button onclick=\"sendCommandFromButton('m')\">Middle All Servos</button>");
  client.println("<button onclick=\"sendCommandFromButton('p')\">Close All Servos</button>");
  // Electromagnet 1 buttons
  client.println("<h3>Electromagnet 1</h3>");
  client.println("<button onclick=\"sendCommandFromButton('1')\">Turn On</button>");
  client.println("<button onclick=\"sendCommandFromButton('2')\">Turn Off</button>");
  // Electromagnet 2 buttons
  client.println("<h3>Electromagnet 2</h3>");
  client.println("<button onclick=\"sendCommandFromButton('3')\">Turn On</button>");
  client.println("<button onclick=\"sendCommandFromButton('4')\">Turn Off</button>");
  client.println("</div>");

  // Current Status Section (Right Side)
  client.println("<div class='panel panel-half'>");
  client.println("<h2>Current Status</h2>");
  client.println("<p><strong>Servo 9 Position:</strong> <span id='servo9Status'>Loading...</span></p>");
  client.println("<p><strong>Servo 10 State:</strong> <span id='servo10Status'>Loading...</span></p>");
  client.println("<p><strong>Servo 11 State:</strong> <span id='servo11Status'>Loading...</span></p>");
  client.println("<p><strong>Electromagnet 1 State:</strong> <span id='electro1Status'>Loading...</span></p>");
  client.println("<p><strong>Electromagnet 2 State:</strong> <span id='electro2Status'>Loading...</span></p>");
  client.println("</div>");

  // End side-by-side section
  client.println("</div>");

  client.println("</body></html>");
}

// Function to process commands
void processCommand(String command) {
  command.trim(); // Remove whitespace
  command.toLowerCase(); // Convert to lowercase for case-insensitive comparison

  if (command.length() != 1) {
    Serial.println("Invalid command length. Only single-key commands are accepted.");
    return;
  }

  char cmd = command.charAt(0);

  switch (cmd) {
    // Servo 9 Control
    case 'q':
      servo9.write(MIN_ANGLE);
      pos9 = MIN_ANGLE;
      Serial.println("Servo 9 moved to 0°");
      break;
    case 'w':
      servo9.write(90);
      pos9 = 90;
      Serial.println("Servo 9 moved to 90°");
      break;
    case 'e':
      servo9.write(MAX_ANGLE);
      pos9 = MAX_ANGLE;
      Serial.println("Servo 9 moved to 180°");
      break;

    // Servo 10 Control (Axon Mini)
    case 'a': // Down 3s
      startServo10Movement(SERVO10_DOWN_SPEED, 3000, 1);
      break;
    case 'b': // Down 1s
      startServo10Movement(SERVO10_DOWN_SPEED, 1000, 1);
      break;
    case 'c': // Down 0.5s
      startServo10Movement(SERVO10_DOWN_SPEED, 500, 1);
      break;
    case 'd': // Up 3s
      startServo10Movement(SERVO10_UP_SPEED, 3000, -1);
      break;
    case 'n': // Up 1s (changed from 'e' to 'n')
      startServo10Movement(SERVO10_UP_SPEED, 1000, -1);
      break;
    case 'f': // Up 0.5s
      startServo10Movement(SERVO10_UP_SPEED, 500, -1);
      break;

    // Servo 11 Control (Axon Mini)
    case 'g': // Down 3s
      startServo11Movement(SERVO11_DOWN_SPEED, 3000, 1);
      break;
    case 'h': // Down 1s
      startServo11Movement(SERVO11_DOWN_SPEED, 1000, 1);
      break;
    case 'i': // Down 0.5s
      startServo11Movement(SERVO11_DOWN_SPEED, 500, 1);
      break;
    case 'j': // Up 3s
      startServo11Movement(SERVO11_UP_SPEED, 3000, -1);
      break;
    case 'k': // Up 1s
      startServo11Movement(SERVO11_UP_SPEED, 1000, -1);
      break;
    case 'l': // Up 0.5s
      startServo11Movement(SERVO11_UP_SPEED, 500, -1);
      break;

    // All Servos Control
    case 'o': // Open All
      moveAllServos(MAX_ANGLE);
      Serial.println("All servos set to open.");
      break;
    case 'm': // Middle All
      moveAllServos(90);
      Serial.println("All servos set to middle position.");
      break;
    case 'p': // Close All
      moveAllServos(MIN_ANGLE);
      Serial.println("All servos set to close.");
      break;

    // Electromagnet 1 (Pin 13)
    case '1': // Turn On
      pin13State = HIGH;
      digitalWrite(13, pin13State);
      Serial.println("Electromagnet 1 turned On.");
      break;
    case '2': // Turn Off
      pin13State = LOW;
      digitalWrite(13, pin13State);
      Serial.println("Electromagnet 1 turned Off.");
      break;

    // Electromagnet 2 (Pin 8)
    case '3': // Turn On
      pin8State = HIGH;
      digitalWrite(8, pin8State);
      Serial.println("Electromagnet 2 turned On.");
      break;
    case '4': // Turn Off
      pin8State = LOW;
      digitalWrite(8, pin8State);
      Serial.println("Electromagnet 2 turned Off.");
      break;

    default:
      Serial.println("Unknown command.");
      break;
  }
}

// Helper function to start Servo 10 movement
void startServo10Movement(int speed, unsigned long duration, int direction) {
  servo10.write(speed);
  servo10Moving = true;
  servo10Direction = direction;
  servo10MoveStartTime = millis();
  servo10MoveDuration = duration;
  Serial.print("Servo 10 moving ");
  Serial.print(direction == 1 ? "down" : "up");
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println(" ms.");
}

// Helper function to start Servo 11 movement
void startServo11Movement(int speed, unsigned long duration, int direction) {
  servo11.write(speed);
  servo11Moving = true;
  servo11Direction = direction;
  servo11MoveStartTime = millis();
  servo11MoveDuration = duration;
  Serial.print("Servo 11 moving ");
  Serial.print(direction == 1 ? "down" : "up");
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println(" ms.");
}

// Function to send status as JSON
void sendStatus(WiFiClient& client) {
  // Prepare JSON data
  String json = "{";
  json += "\"pos9\":" + String(pos9) + ",";
  // Servo 10 state
  String servo10State = servo10Moving ? (servo10Direction == 1 ? "Moving Down" : "Moving Up") : "Stopped";
  json += "\"servo10State\":\"" + servo10State + "\",";
  // Servo 11 state
  String servo11State = servo11Moving ? (servo11Direction == 1 ? "Moving Down" : "Moving Up") : "Stopped";
  json += "\"servo11State\":\"" + servo11State + "\",";
  json += "\"electro1\":\"" + String(pin13State == HIGH ? "On" : "Off") + "\",";
  json += "\"electro2\":\"" + String(pin8State == HIGH ? "On" : "Off") + "\"";
  json += "}";

  // Send HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(json);
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
  // Move servo 9
  servo9.write(angle);
  pos9 = angle;

  // For servos 10 and 11 (Axon Minis), handle appropriately
  if (angle == MAX_ANGLE) {
    // Open All - Start moving down for 3 seconds
    startServo10Movement(SERVO10_DOWN_SPEED, 3000, 1);
    startServo11Movement(SERVO11_DOWN_SPEED, 3000, 1);
  } else if (angle == MIN_ANGLE) {
    // Close All - Start moving up for 3 seconds
    startServo10Movement(SERVO10_UP_SPEED, 3000, -1);
    startServo11Movement(SERVO11_UP_SPEED, 3000, -1);
  } else if (angle == 90) {
    // Middle All - Stop servos 10 and 11
    servo10.write(SERVO10_STOP_ANGLE);
    servo10Moving = false;
    servo11.write(SERVO11_STOP_ANGLE);
    servo11Moving = false;
    Serial.println("Servos 10 and 11 stopped.");
  }
}
