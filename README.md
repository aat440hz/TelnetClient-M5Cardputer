M5Cardputer Telnet Client

This is a simple Telnet client example for the M5Cardputer, an ESP32-based development board with a built-in screen and keyboard. This sketch allows you to connect to a Telnet server over Wi-Fi and send/receive text data.

-Prerequisites

Before using this sketch, make sure you have the following:

An M5Cardputer board.
Arduino IDE installed with the necessary libraries.
Access to a Wi-Fi network.
A Telnet server to connect to.
Getting Started
Open the Arduino IDE.

Install the required libraries if you haven't already. You can install them using the Arduino Library Manager. Search for and install the following libraries:

WiFi
M5Cardputer
M5GFX
Upload the provided sketch (M5Cardputer_Telnet_Client.ino) to your M5Cardputer board.

Replace the following placeholders in the sketch with your Wi-Fi and Telnet server information:

cpp
Copy code
const char* ssid = "Your_SSID";  // Replace with your WiFi SSID
const char* password = "Your_Password";  // Replace with your WiFi password

const char* host = "Your_Telnet_Server_IP"; // Replace with your Telnet server address
const uint16_t port = 23; // Telnet default port
Upload the modified sketch to your M5Cardputer board.

The M5Cardputer should connect to the specified Wi-Fi network and the Telnet server.

Use the M5Cardputer's keyboard to input text. Press keys to type, use the Enter key to send a message to the Telnet server, and use the Del key to delete characters.

-How It Works

This sketch initializes the M5Cardputer, connects to the Wi-Fi network, and establishes a Telnet connection to the specified server. It then continuously monitors the keyboard for input and sends messages to the Telnet server when the Enter key is pressed.

Received messages from the Telnet server are displayed on the M5Cardputer's screen.

https://github.com/aat440hz/TelnetServer-M5StickCPlus

https://github.com/aat440hz/RF433-TelnetChatMorseCode-M5StickCPlus

https://github.com/aat440hz/RF433-TelnetChat-M5StickCPlus

-License

This project is licensed under the MIT License - see the LICENSE file for details.

-Acknowledgments

This sketch was created for educational purposes and is based on the M5Cardputer library.
M5Stack - https://github.com/m5stack/M5Cardputer
Feel free to customize and extend this sketch according to your project requirements.

-Telnet Servers

Here is a list of Telnet servers online you can test this script out with!

horizons.jpl.nasa.gov port 6775

telehack.com port 23

resort.org port 2323

