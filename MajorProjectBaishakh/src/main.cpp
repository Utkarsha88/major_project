
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/* ---------------- BME680 ---------------- */
#define SDA_PIN 21
#define SCL_PIN 22

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;

/* ---------------- TCS3200 ---------------- */
#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

/* ---------------- COLOR CALIBRATION ---------------- */
/* Adjust according to your setup */
int redMin = 60, redMax = 400;
int greenMin = 65, greenMax = 450;
int blueMin = 50, blueMax = 350;

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

  Serial.println("BME680 + TCS3200 Test");

  /* -------- BME680 -------- */
  if (!bme.begin(0x76)) {

    if (!bme.begin(0x77)) {

      Serial.println("Could not find BME680 sensor!");
      while (1);
    }
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

    Serial.println("Failed to perform reading");
    return;
  }

  /* -------- TCS3200 -------- */
  int redRaw = averageRead(LOW, LOW);

  int greenRaw = averageRead(HIGH, HIGH);

  int blueRaw = averageRead(LOW, HIGH);

  /* -------- MAP TO 0-255 -------- */
  int R = mapColor(redRaw, redMin, redMax);

  int G = mapColor(greenRaw, greenMin, greenMax);

  int B = mapColor(blueRaw, blueMin, blueMax);

  /* -------- OUTPUT -------- */

  Serial.print("R: ");
  Serial.print(R);

  Serial.print("  G: ");
  Serial.print(G);

  Serial.print("  B: ");
  Serial.println(B);

  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" °C");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Gas Resistance = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println("-------------------------");

  delay(2000);
}