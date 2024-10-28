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

// Current positions of servos
int pos9 = 90;
int pos10 = 90;
int pos11 = 90;

// Define angle constants
const int MIN_ANGLE = 0;
const int CENTER_ANGLE = 90;
const int MAX_ANGLE = 180;

// States of pin 13 and pin 8
int pin13State = LOW; // Electromagnet 1 (Pin 13)
int pin8State = LOW;  // Electromagnet 2 (Pin 8)

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
    client.println("<li><strong>Servo 11:</strong> 'z' (0&deg;), 'x' (90&deg;), 'c' (180&deg;)</li>");
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
    // Servo 11 buttons
    client.println("<h3>Servo 11</h3>");
    client.println("<button onclick=\"sendCommandFromButton('z')\">Close (0&deg;)</button>");
    client.println("<button onclick=\"sendCommandFromButton('x')\">Middle (90&deg;)</button>");
    client.println("<button onclick=\"sendCommandFromButton('c')\">Open (180&deg;)</button>");
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
    client.println("<p><strong>Servo 9 Position:</strong> " + String(pos9) + "&deg;</p>");
    client.println("<p><strong>Servo 10 Position:</strong> " + String(pos10) + "&deg;</p>");
    client.println("<p><strong>Servo 11 Position:</strong> " + String(pos11) + "&deg;</p>");
    client.println("<p><strong>Electromagnet 1 State:</strong> " + String(pin13State == HIGH ? "On" : "Off") + "</p>");
    client.println("<p><strong>Electromagnet 2 State:</strong> " + String(pin8State == HIGH ? "On" : "Off") + "</p>");
    client.println("</div>");

    // End side-by-side section
    client.println("</div>");

    // JavaScript function to handle button commands
    client.println("<script>");
    client.println("function sendCommandFromButton(cmd) {");
    client.println("  document.getElementById('commandInput').value = cmd;");
    client.println("  sendCommand();");
    client.println("}");
    client.println("</script>");

    client.println("</body></html>");

    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
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

    // Servo 11 Control
    case 'z':
      servo11.write(MIN_ANGLE);
      pos11 = MIN_ANGLE;
      Serial.println("Servo 11 moved to 0°");
      break;
    case 'x':
      servo11.write(CENTER_ANGLE);
      pos11 = CENTER_ANGLE;
      Serial.println("Servo 11 moved to 90°");
      break;
    case 'c':
      servo11.write(MAX_ANGLE);
      pos11 = MAX_ANGLE;
      Serial.println("Servo 11 moved to 180°");
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
}
