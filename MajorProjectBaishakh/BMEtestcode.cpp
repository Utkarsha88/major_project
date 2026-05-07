#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);

  if (!bme.begin()) {
    Serial.println("Could not find BME680!");
    while (1);
  }

  // Optional settings
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setGasHeater(320, 150); // 320°C for 150 ms
}

void loop() {

  if (!bme.performReading()) {
    Serial.println("Failed reading");
    return;
  }

  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas Resistance = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.println("-------------------");

  delay(2000);
}