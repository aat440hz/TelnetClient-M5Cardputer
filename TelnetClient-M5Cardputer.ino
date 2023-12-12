#include <WiFi.h>
#include "M5Cardputer.h"

const char* ssid = "Your_SSID";  // Replace with your WiFi SSID
const char* password = "Your_Password";  // Replace with your WiFi password

WiFiClient telnetClient;
const char* host = "telehack.com"; // Replace with your Telnet server address
const uint16_t port = 23; // Telnet default port

M5Canvas canvas(&M5Cardputer.Display);
String data = "> ";

// Add a variable to keep track of the Y position
int cursorY = 0;
const int lineHeight = 8;  // Height of a text line, adjust as needed

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

    // Handle keyboard input
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
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

    // Check if 'c' is an ASCII character
    if (c >= 0 && c <= 127) {
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
