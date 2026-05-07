#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

/* =========================
   TCS3200 PINS
========================= */
#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

/* =========================
   BME680
========================= */
Adafruit_BME680 bme;

/* =========================
   COLOR CALIBRATION
========================= */
int redMin = 60;
int redMax = 400;

int greenMin = 65;
int greenMax = 450;

int blueMin = 50;
int blueMax = 350;

/* =========================
   VARIABLES
========================= */
float baselineGas = 0;

float gasHistory[10];
int gasIndex = 0;

/* =========================
   READ COLOR
========================= */
int readColor(bool s2, bool s3) {

  digitalWrite(S2, s2);
  digitalWrite(S3, s3);

  delay(300);

  return pulseIn(sensorOut, LOW);
}

/* =========================
   AVERAGE COLOR
========================= */
int averageColor(bool s2, bool s3) {

  long sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += readColor(s2, s3);
  }

  return sum / 10;
}

/* =========================
   SMOOTH GAS
========================= */
float smoothGas(float value) {

  gasHistory[gasIndex] = value;

  gasIndex++;

  if (gasIndex >= 10) {
    gasIndex = 0;
  }

  float total = 0;

  for (int i = 0; i < 10; i++) {
    total += gasHistory[i];
  }

  return total / 10.0;
}

/* =========================
   SETUP
========================= */
void setup() {

  Serial.begin(115200);

  /* ----- TCS3200 ----- */

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(sensorOut, INPUT);

  // 20% scaling
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  /* ----- BME680 ----- */

  if (!bme.begin()) {

    Serial.println("BME680 not found");

    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);

  bme.setGasHeater(320, 150);

  Serial.println("Warming up...");
  delay(10000);

  /* ----- BASELINE ----- */

  Serial.println("Remove fruit");
  delay(5000);

  float total = 0;

  for (int i = 0; i < 180; i++) {

    bme.performReading();
    Serial.print(bme.gas_resistance / 1000.0);

    float gasValue = bme.gas_resistance / 1000.0;

    total += gasValue;

    delay(1000);
  }

  baselineGas = total / 180.0;

  Serial.print("Baseline Gas: ");
  Serial.print(baselineGas);
  Serial.println(" KOhm");

  Serial.println("Place fruit");
  delay(15000);

  /* The code snippet `for (int i = 0; i < 10; i++) { gasHistory[i] = baselineGas; }` is initializing
  the `gasHistory` array with the `baselineGas` value. It is setting each element of the
  `gasHistory` array to the `baselineGas` value in a loop that runs 10 times. This initialization is
  done to ensure that the `gasHistory` array starts with consistent values before it is used for
  storing gas sensor readings in the `smoothGas` function. */
  for (int i = 0; i < 10; i++) {
    gasHistory[i] = baselineGas;
  }
}

/* =========================
   LOOP
========================= */
void loop() {

  /* ----- RGB ----- */

  int redFreq = averageColor(LOW, LOW);
  int greenFreq = averageColor(HIGH, HIGH);
  int blueFreq = averageColor(LOW, HIGH);

  int R = map(redFreq, redMin, redMax, 255, 0);
  int G = map(greenFreq, greenMin, greenMax, 255, 0);
  int B = map(blueFreq, blueMin, blueMax, 255, 0);

  R = constrain(R, 0, 255);
  G = constrain(G, 0, 255);
  B = constrain(B, 0, 255);

  /* ----- BME680 ----- */

  if (!bme.performReading()) {

    Serial.println("Reading failed");
    return;
  }

  float temperature = bme.temperature;
  float humidity = bme.humidity;

  float rawGas = bme.gas_resistance / 1000.0;

  float gasNow = smoothGas(rawGas);

  float vocPercent =
    ((baselineGas - gasNow) / baselineGas) * 100.0;

  if (vocPercent < 0) {
    vocPercent = 0;
  }

  /* ----- OUTPUT ----- */

  Serial.print("R:");
  Serial.print(R);

  Serial.print(" G:");
  Serial.print(G);

  Serial.print(" B:");
  Serial.println(B);

  Serial.print("Temperature:");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity:");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("BaselineGas:");
  Serial.print(baselineGas);
  Serial.println(" KOhm");

  Serial.print("CurrentGas:");
  Serial.print(gasNow);
  Serial.println(" KOhm");

  Serial.print("VOCChange:");
  Serial.print(vocPercent);
  Serial.println(" %");

  Serial.println();

  delay(1000);
}