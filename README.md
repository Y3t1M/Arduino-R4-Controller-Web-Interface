# Arduino-R4-Controller-Web-Interface
This project enables remote control of multiple servos and electromagnets via a web-based interface hosted on an Arduino R4 WiFi board. Users can access the control panel over WiFi to operate servos connected to pins 9, 10, and 11 and electromagnets connected to pins 13 and 8.
## Web-Based Control Panel

- **Accessible via WiFi:** No additional apps required.
- **Individual Movement:** Control servos on pins 9, 10, and 11 to move to Closed (0°), Middle (90°), or Open (180°) positions.
- **Simultaneous Movement:** Open or close all servos at once with dedicated buttons.
- **Electromagnet Control:**
    - **Electromagnet 1 (Pin 13):** Turn on/off via the web interface.
    - **Electromagnet 2 (Pin 8):** Turn on/off via the web interface.
- **Real-Time Status:** Displays current positions and states on the web page.
- **Stylish Interface:** Cyan-colored buttons and a user-friendly layout.

## Hardware Requirements

- **Arduino R4 WiFi Board**
- **Breadboard** for wire and power management
- **Servos:** 3 standard hobby servos connected to pins 9, 10, and 11.
- **Electromagnets:**
    - **Electromagnet 1:** Connected to pin 13.
    - **Electromagnet 2:** Connected to pin 8.
- **Power Supply:** High-voltage power supply for Arduino
- **Miscellaneous:** Wires, resistors, and other standard electronic components.

## Software Requirements

- **Arduino IDE:** Compatible with Arduino R4 WiFi.
- **Libraries:**
    - **WiFiS3:** For WiFi functionality.
    - **Servo:** For controlling servos.

## Setup Instructions

### Hardware Setup
Plug the Arduino R4 WiFi board into your computer via USB if you intend to upload code. If the code has already been uploaded, plug it into your external power supply.

**Servos:**

- Connect servo signal wires to pins 9, 10, and 11.
- Connect servo power and ground wires to respective slots on breadboard.

**Electromagnets:**

- Connect VCC and GND pins to respective slots on breadboard.
- Connect electromagnet SIG pins to Arduino digital pins 13 and 8.

### Software Setup

1. Clone or download this repository.
2. Open the sketch in the Arduino IDE.
3. **Configure WiFi Access Point (optional):**
     - Change `SSID` and `pass` variables in the code to modify the network name and password.
4. Upload the code to your Arduino R4 WiFi board.

## Usage Instructions

### Connect to WiFi Network

- Connect your computer or mobile device to the WiFi network `RVR_Network` (or your custom SSID).
- Enter the password `123456789` (or your custom password).

### Access the Control Panel

- Open a web browser.
- Navigate to `http://192.168.4.1` (default IP address for the Arduino access point).

### Operate Servos

**Individual Control:**

- Under each servo section, click:
    - **Close** to move to 0°.
    - **Middle** to move to 90°.
    - **Open** to move to 180°.

**All Servos Control:**

- Click **Open All Servos** or **Close All Servos** to move all servos simultaneously.

### Operate Electromagnets

**Electromagnet 1 (Pin 13):**

- Click **Turn On** or **Turn Off** to control.

**Electromagnet 2 (Pin 8):**

- Click **Turn On** or **Turn Off** to control.

### Monitor Status

- The web page displays the current position of each servo and the state (On/Off) of each electromagnet.

## Safety Precautions

### Ground Connections

- Ensure all components share a common ground to prevent floating grounds and potential damage.

### Testing

- Test components individually before integrating into the full system.

## Customization

- **Adjust Servo Angles:** Modify `MIN_ANGLE`, `CENTER_ANGLE`, and `MAX_ANGLE` in the code to suit your servos.
- **Change WiFi Credentials:** Update the `ssid` and `pass` variables in the code.
- **Expand Functionality:** Add more servos or outputs by replicating the code sections and adjusting pin numbers.
