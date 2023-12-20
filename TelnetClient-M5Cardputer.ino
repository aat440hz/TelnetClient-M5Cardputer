#include <WiFi.h>
#include "M5Cardputer.h"

// WiFi and Telnet configurations
const char* ssid = "Your_SSID";  // Replace with your WiFi SSID
const char* password = "Your_Password";  // Replace with your WiFi password
const char* host = "telehack.com"; // Replace with your Telnet server address
const uint16_t port = 23; // Telnet default port

// M5Cardputer setup
M5Canvas canvas(&M5Cardputer.Display);
String data = "> ";
int cursorY = 0;
const int lineHeight = 8;
unsigned long lastKeyPressMillis = 0;
const unsigned long debounceDelay = 200; // Adjust debounce delay as needed

// Telnet Command Codes
const byte IAC = 255;
const byte DO = 253;
const byte DONT = 254;
const byte WILL = 251;
const byte WONT = 252;

WiFiClient telnetClient;

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(1); // Set text size

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // Connect to Telnet server
    if (!telnetClient.connect(host, port)) {
        // Handle connection failure
    }

    // Initialize the cursor Y position
    cursorY = M5Cardputer.Display.getCursorY();
}

void loop() {
    M5Cardputer.update();

    // Handle keyboard input with debounce
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastKeyPressMillis >= debounceDelay) {
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

                for (auto i : status.word) {
                    data += i;
                    M5Cardputer.Display.print(i); // Display the character as it's typed
                    cursorY = M5Cardputer.Display.getCursorY(); // Update cursor Y position
                }

                if (status.del && data.length() > 2) {
                    data.remove(data.length() - 1);
                    M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 6, M5Cardputer.Display.getCursorY());
                    M5Cardputer.Display.print(" "); // Print a space to erase the last character
                    M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 6, M5Cardputer.Display.getCursorY());
                    cursorY = M5Cardputer.Display.getCursorY(); // Update cursor Y position
                }

                if (status.enter) {
                    String message = data.substring(2) + "\r\n"; // Use "\r\n" for newline
                    telnetClient.write(message.c_str());  // Send message to Telnet server

                    data = "> ";
                    M5Cardputer.Display.print('\n'); // Move to the next line
                    cursorY = M5Cardputer.Display.getCursorY(); // Update cursor Y position
                }

                lastKeyPressMillis = currentMillis;
            }
        }
    }

    // Check if the cursor has reached the bottom of the display
    if (cursorY > M5Cardputer.Display.height() - lineHeight) {
        // Scroll the display up by one line
        M5Cardputer.Display.scroll(0, -lineHeight);

        // Reset the cursor to the new line position
        cursorY -= lineHeight;
        M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX(), cursorY);
    }

    // Read data from Telnet server and display it on the screen
    while (telnetClient.available()) {
        char c = telnetClient.read();

        if (c == IAC) {
            handleTelnetCommand();
        } else if (c >= 0 && c <= 127) {
            M5Cardputer.Display.print(c);
            cursorY = M5Cardputer.Display.getCursorY(); // Update cursor Y position
        }

        // Scroll the display if needed
        if (cursorY > M5Cardputer.Display.height() - lineHeight) {
            M5Cardputer.Display.scroll(0, -lineHeight);
            cursorY -= lineHeight;
            M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX(), cursorY);
        }
    }
}

void handleTelnetCommand() {
    byte command = telnetClient.read();
    byte option = telnetClient.read();

    switch (command) {
        case DO:
        case DONT:
            respondToCommand(WONT, option);
            break;
        case WILL:
        case WONT:
            respondToCommand(DONT, option);
            break;
    }
}

void respondToCommand(byte response, byte option) {
    telnetClient.write(IAC);
    telnetClient.write(response);
    telnetClient.write(option);
}
