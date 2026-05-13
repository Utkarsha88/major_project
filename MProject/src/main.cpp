#include <Arduino.h>

void setup() {
    // Initialize serial communication at 115200 baud
    Serial.begin(115200);
    while (!Serial) { ; }  // Wait for Serial port to be ready (optional)
    Serial.println("ESP32 PlatformIO Project Started!");
}

void loop() {
    // Blink the built-in LED (usually GPIO 2 on ESP32 Dev Module)
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on
    delay(1000);                        // Wait 1 second
    digitalWrite(LED_BUILTIN, LOW);    // Turn the LED off
    delay(1000);                        // Wait 1 second

    // Print a message to the Serial Monitor
    Serial.println("Blinking LED...");
}
