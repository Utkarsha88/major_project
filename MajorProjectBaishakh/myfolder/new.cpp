
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/* ---------------- BME680 ---------------- */
#define SDA_PIN 21
#define SCL_PIN 22

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme(&Wire);

/* ---------------- TCS3200 ---------------- */
#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

/* ---------------- COLOR CALIBRATION ---------------- */
/* Adjust these after testing */
int redMin = 70, redMax = 400;
int greenMin = 70, greenMax = 450;
int blueMin = 60, blueMax = 350;

/* ---------------- FUNCTIONS ---------------- */

int readColor(bool s2, bool s3) {

  digitalWrite(S2, s2);
  digitalWrite(S3, s3);

  delay(20);

  return pulseIn(sensorOut, LOW);
}

int averageRead(bool s2, bool s3) {

  long sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += readColor(s2, s3);
  }

  return sum / 10;
}

int mapColor(int value, int minVal, int maxVal) {

  int mapped = map(value, minVal, maxVal, 255, 0);

  return constrain(mapped, 0, 255);
}

/* ---------------- SETUP ---------------- */

void setup() {

  Serial.begin(115200);

  /* -------- I2C -------- */
  Wire.begin(SDA_PIN, SCL_PIN);

  /* -------- TCS3200 -------- */
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  /* Frequency scaling 20% */
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("BME680 + TCS3200 TEST");

  /* -------- BME680 -------- */
  if (!bme.begin()) {

    Serial.println("Could not find BME680 sensor!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);

  bme.setHumidityOversampling(BME680_OS_2X);

  bme.setPressureOversampling(BME680_OS_4X);

  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  bme.setGasHeater(320, 150);

  Serial.println("Sensors Ready");
}

/* ---------------- LOOP ---------------- */

void loop() {

  /* -------- BME680 -------- */
  if (!bme.performReading()) {

    Serial.println("Failed BME680 reading");
    return;
  }

  /* -------- TCS3200 RAW -------- */
  int redRaw = averageRead(LOW, LOW);

  int greenRaw = averageRead(HIGH, HIGH);

  int blueRaw = averageRead(LOW, HIGH);

  /* -------- MAPPED RGB 0-255 -------- */
  int R = mapColor(redRaw, redMin, redMax);

  int G = mapColor(greenRaw, greenMin, greenMax);

  int B = mapColor(blueRaw, blueMin, blueMax);

  /* -------- OUTPUT -------- */

  
  Serial.printf("%d,%d,%d,%.2f,%.2f,%.2f", R, G, B, bme.temperature, bme.humidity, bme.gas_resistance / 1000.0);



Serial.println();

  delay(2000);
}