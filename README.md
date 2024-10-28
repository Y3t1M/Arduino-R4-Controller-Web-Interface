This project enables remote control of multiple servos and electromagnets via a web-based interface hosted on an Arduino R4 WiFi board. Users can access the control panel over WiFi to operate servos connected to pins 9, 10, and 11 and electromagnets connected to pins 13 and 8.

Web-Based Control Panel: Accessible via WiFi, no additional apps required.
Individual Movement: Control servos on pins 9, 10, and 11 to move to Close (0°), Middle (90°), or Open (180°) positions.
Simultaneous Movement: Open or close all servos at once with dedicated buttons.
Electromagnet Control:
Electromagnet 1 (Pin 13): Turn on/off via the web interface.
Electromagnet 2 (Pin 8): Turn on/off via the web interface.
Real-Time Status: Displays current positions and states on the web page.
Stylish Interface: Cyan-colored buttons and a user-friendly layout.

Hardware Requirements
Arduino R4 WiFi Board
Servos: 3 standard hobby servos connected to pins 9, 10, and 11.
Electromagnets:
Electromagnet 1: Connected to pin 13.
Electromagnet 2: Connected to pin 8.

Power Supply:
External Power Source: Recommended for servos and electromagnets. ( I used my laptop for a bit becuase it was convenient but if i had too many things plugged in it wouldnt work becuase it was over drawing so i had to swicth to plugging it into the wall.)

Driver Components:
Transistors/MOSFETs: For switching electromagnets.
Flyback Diodes: Across electromagnet coils.

Miscellaneous:
Breadboard, wires, resistors, and other standard electronic components.
Software Requirements
Arduino IDE: Compatible with Arduino R4 WiFi.

Libraries:
WiFiS3: For WiFi functionality.
Servo: For controlling servos.
Setup Instructions
Hardware Setup

Servos:

Connect servo signal wires to pins 9, 10, and 11.
Connect servo power and ground wires to an external 5V power supply.
Electromagnets:

Use appropriate driver circuits (e.g., N-MOSFETs) connected to pins 13 and 8.
Connect flyback diodes across the electromagnet terminals.
Power electromagnets with an external power supply match their voltage and current requirements.
Common Ground:

Connect the grounds of the Arduino, servos, electromagnets, and external power supplies.
Software Setup
Clone or Download this repository.

Open the Sketch in the Arduino IDE.

Configure WiFi Access Point (optional):

Change SSID and pass variables in the code to modify the network name and password.
Upload the Code to your Arduino R4 WiFi board.

Usage Instructions
Connect to WiFi Network:

Connect to your computer or mobile device to the WiFi network RVR_Network (or your custom SSID).
Enter the password 123456789 (or your custom password).
Access the Control Panel:

Open a web browser.
Navigate to http://192.168.4.1 (default IP address for the Arduino access point).
Operate Servos:

Individual Control:

Under each servo section, click:
Close to move to 0°.
Middle to move to 90°.
Open to move to 180°.
All Servos Control:

Click Open All Servos or Close All Servos to move all servos simultaneously.
Operate Electromagnets:

Electromagnet 1 (Pin 13):
Click Turn On or Turn Off to control.
Electromagnet 2 (Pin 8):
Click Turn On or Turn Off to control.
Monitor Status:

The web page displays the current position of each servo and the state (On/Off) of each electromagnet.
Safety Precautions
Power Handling:

Do Not Power Servos or Electromagnets Directly from Arduino: Use external power supplies.
Voltage Matching: Ensure voltage levels match device specifications.
Circuit Protection:

Use Flyback Diodes: Protect against voltage spikes from inductive loads.
Proper Transistor Selection: Use logic-level N-MOSFETs for switching.
Heat Dissipation: Ensure transistors/MOSFETs are appropriately rated and cooled.
Ground Connections:

Ensure all components share a common ground to prevent floating grounds and potential damage.
Testing:

Test components individually before integrating into the full system.
Customization
Adjust Servo Angles:

Modify MIN_ANGLE, CENTER_ANGLE, and MAX_ANGLE in the code to suit your servos.
Change WiFi Credentials:

Update the ssid and pass variables in the code.
Expand Functionality:

Add more servos or outputs by replicating the code sections and adjusting pin numbers.
