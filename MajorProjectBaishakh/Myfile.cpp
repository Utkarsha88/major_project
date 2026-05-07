#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// ---------------- TCS3200 PINS ----------------
#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

// ---------------- BME680 ----------------
Adafruit_BME680 bme;

// ---------------- VARIABLES ----------------
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

// ---------------- READ COLOR ----------------
int readColor(bool s2, bool s3) {

  digitalWrite(S2, s2);
  digitalWrite(S3, s3);

  delay(50);

  int frequency = pulseIn(sensorOut, LOW);

  return frequency;
}

// ---------------- AVERAGE FILTER ----------------
int averageColor(bool s2, bool s3) {

  long sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += readColor(s2, s3);
  }

  return sum / 10;
}

void setup() {

  Serial.begin(115200);

  // ---------- TCS3200 SETUP ----------
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // Frequency scaling 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  // ---------- BME680 SETUP ----------
  if (!bme.begin()) {
    Serial.println("BME680 not found!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setGasHeater(320, 150);

  // CSV Header
  Serial.println("R,G,B,Temperature,Humidity,Gas_KOhms");

  delay(2000);
}

void loop() {

  // ---------- READ COLORS ----------
  redValue = averageColor(LOW, LOW);
  greenValue = averageColor(HIGH, HIGH);
  blueValue = averageColor(LOW, HIGH);

  // ---------- READ BME680 ----------
  if (!bme.performReading()) {
    Serial.println("BME680 Reading Failed");
    return;
  }

  float temperature = bme.temperature;
  float humidity = bme.humidity;
  float gas = bme.gas_resistance / 1000.0;

  // ---------- PRINT CSV FORMAT ----------
  Serial.print(redValue);
  Serial.print(",");

  Serial.print(greenValue);
  Serial.print(",");

  Serial.print(blueValue);
  Serial.print(",");

  Serial.print(temperature);
  Serial.print(",");

  Serial.print(humidity);
  Serial.print(",");

  Serial.println(gas);

  delay(1000);
}