
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/* -------- I2C PINS FOR ESP32 -------- */
#define SDA_PIN 21
#define SCL_PIN 22

#define SEALEVELPRESSURE_HPA (1013.25)

/* -------- BME680 I2C -------- */
Adafruit_BME680 bme(&Wire);

void setup() {

  Serial.begin(115200);

  /* -------- START I2C -------- */
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("BME680 Test");

  /* -------- CHECK SENSOR -------- */
  if (!bme.begin()) {
    Serial.println("Could not find BME680 sensor!");
    while (1);
  }

  /* -------- SENSOR SETTINGS -------- */
  bme.setTemperatureOversampling(BME680_OS_8X);

  bme.setHumidityOversampling(BME680_OS_2X);

  bme.setPressureOversampling(BME680_OS_4X);

  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  bme.setGasHeater(320, 150);

  Serial.println("BME680 Ready");
}

void loop() {

  if (!bme.performReading()) {
    Serial.println("Failed reading");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(bme.temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Gas Resistance: ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Altitude: ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println("-------------------------");

  delay(2000);
}