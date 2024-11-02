#include <WiFiS3.h>
#include <Servo.h>

#define JSON_BUFF_SIZE 256

// Set the SSID and Password for the access point
char ssid[] = "RVR_Network";    // Access point name
char pass[] = "123456789";      // Password for WPA2
WiFiServer server(80);          // Create a server on port 80

// Create servo objects
Servo servo9;
Servo servo10;
Servo servo11;

// Current positions of servos
int pos9 = 90;
int pos10 = 90;
// Remove pos11 as it's no longer used for positioning

// Define angle constants
const int MIN_ANGLE = 0;
const int CENTER_ANGLE = 90;
const int MAX_ANGLE = 180;

// States of pin 13 and pin 8
int pin13State = LOW; // Electromagnet 1 (Pin 13)
int pin8State = LOW;  // Electromagnet 2 (Pin 8)

// Variables for Servo 11 movement
bool servo11Moving = false;
unsigned long servo11MoveStartTime = 0;
int servo11Direction = 0; // 1 for forward, -1 for backward

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
  servo11.write(CENTER_ANGLE); // Stop position for continuous rotation servo

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
  // Handle Servo 11 movement timing
  if (servo11Moving) {
    if (millis() - servo11MoveStartTime >= 5000) {
      // Stop Servo 11 after 5 seconds
      servo11.write(CENTER_ANGLE); // Stop position
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
  client.println("      document.getElementById('servo10Status').innerHTML = status.pos10 + '&deg;';");
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
  client.println("<li><strong>Servo 10:</strong> 'a' (0&deg;), 's' (90&deg;), 'd' (180&deg;)</li>");
  client.println("<li><strong>Servo 11 (Axon Mini):</strong> 'z' (Forward 5s), 'x' (Backward 5s)</li>");
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
  client.println("<button onclick=\"sendCommandFromButton('q')\">Close (0&deg;)</button>");
  client.println("<button onclick=\"sendCommandFromButton('w')\">Middle (90&deg;)</button>");
  client.println("<button onclick=\"sendCommandFromButton('e')\">Open (180&deg;)</button>");
  // Servo 10 buttons
  client.println("<h3>Servo 10</h3>");
  client.println("<button onclick=\"sendCommandFromButton('a')\">Close (0&deg;)</button>");
  client.println("<button onclick=\"sendCommandFromButton('s')\">Middle (90&deg;)</button>");
  client.println("<button onclick=\"sendCommandFromButton('d')\">Open (180&deg;)</button>");
  // Servo 11 buttons (Axon Mini)
  client.println("<h3>Servo 11 (Axon Mini)</h3>");
  client.println("<button onclick=\"sendCommandFromButton('z')\">Forward (5s)</button>");
  client.println("<button onclick=\"sendCommandFromButton('x')\">Backward (5s)</button>");
  // All Servos Control
  client.println("<h3>All Servos</h3>");
  client.println("<button onclick=\"sendCommandFromButton('o')\">Open All Servos (180&deg;)</button>");
  client.println("<button onclick=\"sendCommandFromButton('m')\">Middle All Servos (90&deg;)</button>");
  client.println("<button onclick=\"sendCommandFromButton('p')\">Close All Servos (0&deg;)</button>");
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
  client.println("<p><strong>Servo 10 Position:</strong> <span id='servo10Status'>Loading...</span></p>");
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
      servo9.write(CENTER_ANGLE);
      pos9 = CENTER_ANGLE;
      Serial.println("Servo 9 moved to 90°");
      break;
    case 'e':
      servo9.write(MAX_ANGLE);
      pos9 = MAX_ANGLE;
      Serial.println("Servo 9 moved to 180°");
      break;

    // Servo 10 Control
    case 'a':
      servo10.write(MIN_ANGLE);
      pos10 = MIN_ANGLE;
      Serial.println("Servo 10 moved to 0°");
      break;
    case 's':
      servo10.write(CENTER_ANGLE);
      pos10 = CENTER_ANGLE;
      Serial.println("Servo 10 moved to 90°");
      break;
    case 'd':
      servo10.write(MAX_ANGLE);
      pos10 = MAX_ANGLE;
      Serial.println("Servo 10 moved to 180°");
      break;

    // Servo 11 Control (Axon Mini)
    case 'z':
      // Move forward for 5 seconds
      servo11.write(MAX_ANGLE); // Full speed forward
      servo11Moving = true;
      servo11Direction = 1;
      servo11MoveStartTime = millis();
      Serial.println("Servo 11 moving forward for 5 seconds.");
      break;
    case 'x':
      // Move backward for 5 seconds
      servo11.write(MIN_ANGLE); // Full speed backward
      servo11Moving = true;
      servo11Direction = -1;
      servo11MoveStartTime = millis();
      Serial.println("Servo 11 moving backward for 5 seconds.");
      break;

    // All Servos Control
    case 'o': // Open All
      moveAllServos(MAX_ANGLE);
      Serial.println("All servos moved to 180°");
      break;
    case 'm': // Middle All
      moveAllServos(CENTER_ANGLE);
      Serial.println("All servos moved to 90°");
      break;
    case 'p': // Close All
      moveAllServos(MIN_ANGLE);
      Serial.println("All servos moved to 0°");
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

// Function to send status as JSON
void sendStatus(WiFiClient& client) {
  // Prepare JSON data
  String json = "{";
  json += "\"pos9\":" + String(pos9) + ",";
  json += "\"pos10\":" + String(pos10) + ",";
  String servo11State = servo11Moving ? (servo11Direction == 1 ? "Moving Forward" : "Moving Backward") : "Stopped";
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
  // Move servos 9 and 10
  servo9.write(angle);
  servo10.write(angle);
  pos9 = angle;
  pos10 = angle;

  // For servo 11 (Axon Mini), handle appropriately
  if (angle == MAX_ANGLE) {
    // Open All - Start moving forward
    servo11.write(MAX_ANGLE);
    servo11Moving = true;
    servo11Direction = 1;
    servo11MoveStartTime = millis();
  } else if (angle == MIN_ANGLE) {
    // Close All - Start moving backward
    servo11.write(MIN_ANGLE);
    servo11Moving = true;
    servo11Direction = -1;
    servo11MoveStartTime = millis();
  } else if (angle == CENTER_ANGLE) {
    // Middle All - Stop servo 11
    servo11.write(CENTER_ANGLE);
    servo11Moving = false;
    Serial.println("Servo 11 stopped.");
  }
}
