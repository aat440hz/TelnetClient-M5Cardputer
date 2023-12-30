#include <WiFi.h>
#include "M5Cardputer.h"

// WiFi configurations
const char* ssid = "Your_SSID";  // Replace with your WiFi SSID
const char* password = "Your_Password";  // Replace with your WiFi password

// Variables for server address and port
String serverAddress;
uint16_t port = 23; // Default Telnet port

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

// Compile-time flag for ANSI sequence filtering
const bool ansiFilteringEnabled = true;  // Set to true or false based on server requirements

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

    // Prompt for server address and optional port
    M5Cardputer.Display.print("Address:Port: ");
    String serverInput = waitForInput();
    parseServerInput(serverInput); // Parse and set serverAddress and port

    // Connect to Telnet server
    if (!telnetClient.connect(serverAddress.c_str(), port)) {
        // Handle connection failure
        M5Cardputer.Display.println("Failed to connect");
    } else {
        M5Cardputer.Display.println("Connected to " + serverAddress + ":" + String(port));
    }

    // Initialize the cursor Y position
    cursorY = M5Cardputer.Display.getCursorY();
}

void loop() {
    M5Cardputer.update();

    // Check for and handle any user input first
    handleUserInput(); // A new function to encapsulate all user input handling

    // Handle any incoming data from the server
    readAndProcessServerData();
}

String waitForInput() {
    String input = "";
    M5Cardputer.Display.setCursor(90, cursorY); // Set initial cursor position for input
    unsigned long lastKeyPressMillis = 0;
    const unsigned long debounceDelay = 200;

    while (!M5Cardputer.Keyboard.keysState().enter) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            if (status.del && input.length() > 0) {
                input.remove(input.length() - 1);  // Remove last character from input
                // Visually remove character from display:
                M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 6, cursorY);
                M5Cardputer.Display.print(" ");
                M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 6, cursorY);
                lastKeyPressMillis = millis();
            }

            for (auto i : status.word) {
                if (millis() - lastKeyPressMillis >= debounceDelay) {
                    lastKeyPressMillis = millis(); // Update the last key press time

                    if (isPrintable(i)) {
                        input += i; // Append the character to input string
                        M5Cardputer.Display.print(i); // Show the character on display
                    }
                }
            }

            if (status.enter) {
                M5Cardputer.Display.println(); // Move to the next line
                break; // Break the loop as enter has been pressed
            }
        }
    }
    return input;
}

void parseServerInput(String serverInput) {
    int colonIndex = serverInput.indexOf(':');
    if (colonIndex != -1) {
        // Split the input into address and port
        serverAddress = serverInput.substring(0, colonIndex);
        String portStr = serverInput.substring(colonIndex + 1);
        port = (portStr.length() > 0) ? portStr.toInt() : 23; // Use default port if none specified
    } else {
        serverAddress = serverInput; // Entire input is treated as address
        port = 23; // Default to port 23
    }
}

void handleUserInput() {
    bool controlMode = M5Cardputer.BtnA.isPressed();
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastKeyPressMillis >= debounceDelay) {
            lastKeyPressMillis = currentMillis; // Update the last key press time

            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            for (auto i : status.word) {
                if (controlMode) {
                    // If BtnA is pressed, modify the character to a control character
                    char ctrlChar = mapToControlCharacter(i);
                    telnetClient.write(ctrlChar);  // Send control character to Telnet server

                    // Display the conventional representation for control characters (e.g., '^C')
                    M5Cardputer.Display.print('^');
                    M5Cardputer.Display.print(toupper(i)); // Display the uppercase letter
                } else {
                    // Normal character handling
                    data += i; // Add character to the data buffer
                    M5Cardputer.Display.print(i); // Display the character
                }
                cursorY = M5Cardputer.Display.getCursorY(); // Update cursor Y position
            }

            if (status.del && data.length() > 2) {
                data.remove(data.length() - 1);
                M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 6, M5Cardputer.Display.getCursorY());
                M5Cardputer.Display.print(" ");
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
}

void readAndProcessServerData() {
    bool isAnsiSequence = false; // Track if currently processing an ANSI sequence
    while (telnetClient.available()) {
        char c = telnetClient.read();

        if (c == IAC) {
            // Handle Telnet command received from the server
            handleTelnetCommand();
        } else if (ansiFilteringEnabled && c == '\033') { // Start of an ANSI sequence
            isAnsiSequence = true;
        } else if (isAnsiSequence) {
            if (isalpha(c) || c == '@') { // End of an ANSI sequence ('@' is included for edge cases)
                isAnsiSequence = false; // End of ANSI sequence
            }
            // Don't print ANSI sequence characters
        } else {
            M5Cardputer.Display.print(c);
            cursorY = M5Cardputer.Display.getCursorY(); // Update cursor Y position
        }

        // Check for display scrolling
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

char mapToControlCharacter(char key) {
    // Map alphabetic characters to ASCII control characters (e.g., Ctrl + A to 1)
    if (key >= 'a' && key <= 'z') {
        return key - 'a' + 1; // 'a' maps to 1 ('Ctrl+A'), 'z' maps to 26 ('Ctrl+Z')
    } else if (key >= 'A' && key <= 'Z') {
        return key - 'A' + 1; // 'A' maps to 1 ('Ctrl+A'), 'Z' maps to 26 ('Ctrl+Z')
    } else {
        // Return the original key if it's not alphabetic
        return key;
    }
}
