#include <WiFi.h>
#include "M5Cardputer.h"
#include "M5GFX.h"

const char* ssid = "Telnet_AP";  // Replace with your WiFi SSID
const char* password = "66666666";  // Replace with your WiFi password

WiFiClient telnetClient;
const char* host = "192.168.4.1"; // Replace with your Telnet server address
const uint16_t port = 23; // Telnet default port

M5Canvas canvas(&M5Cardputer.Display);
String data = "> ";

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(0.5);

    // Remove the following lines to eliminate the green outline
    // M5Cardputer.Display.drawRect(0, 0, M5Cardputer.Display.width(), M5Cardputer.Display.height() - 28, GREEN);
    // M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 4, M5Cardputer.Display.width(), 4, GREEN);

    M5Cardputer.Display.setTextFont(&fonts::FreeMono12pt7b);

    canvas.setTextFont(&fonts::FreeMono12pt7b);
    canvas.setTextSize(0.5);
    canvas.createSprite(M5Cardputer.Display.width() - 8, M5Cardputer.Display.height() - 36);
    canvas.setTextScroll(true);
    canvas.pushSprite(4, 4);
    M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // Connect to Telnet server
    if (!telnetClient.connect(host, port)) {
        // Handle connection failure
    }
}

void loop() {
    M5Cardputer.update();

    // Handle keyboard input
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            for (auto i : status.word) {
                data += i;
            }

            if (status.del && data.length() > 2) {
                data.remove(data.length() - 1);
            }

            if (status.enter) {
                String message = data.substring(2) + "\n"; // Add a newline character to the message
                telnetClient.write(message.c_str());  // Send message to Telnet server

                // Display sent message on canvas
                canvas.println("> " + message);
                canvas.pushSprite(4, 4); // Update the display with new canvas content

                data = "> ";
            }

            M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28, M5Cardputer.Display.width(), 25, BLACK);
            M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);
        }
    }

    // Read data from Telnet server and display it on canvas
    while (telnetClient.available()) {
        char c = telnetClient.read();
        canvas.print(c); // Display received character on canvas

        // Check for newline character to refresh the canvas
        if (c == '\n') {
            canvas.pushSprite(4, 4); // Update the display with new canvas content
        }
    }
}
