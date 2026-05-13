#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/* ---------------- BME680 ---------------- */

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BME680 bme;

/* ---------------- TCS3200 ---------------- */

#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

/* ---------------- COLOR CALIBRATION ---------------- */

int redMin = 60, redMax = 400;
int greenMin = 65, greenMax = 450;
int blueMin = 50, blueMax = 350;

/* ---------------- GLOBAL VARIABLES ---------------- */

float temperature = 0;
float humidity = 0;
float pressure = 0;
float gas = 0;

float baselineGas = 0;
float gasDifference = 0;

/* ---------------- CONTROL VARIABLES ---------------- */

bool baselineDone = false;
bool fruitPlaced = false;

unsigned long baselineStart = 0;
unsigned long fruitStart = 0;

/* ---------------- FUNCTIONS ---------------- */

void updateBME()
{
  if (bme.performReading())
  {
    temperature = bme.temperature;

    humidity = bme.humidity;

    pressure = bme.pressure / 100.0;

    gas = bme.gas_resistance / 1000.0;
  }
}

/* ---------------- TCS3200 ---------------- */

int readColor(bool s2, bool s3)
{
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);

  delay(20);

  return pulseIn(sensorOut, LOW);
}

int averageRead(bool s2, bool s3)
{
  long sum = 0;

  for (int i = 0; i < 10; i++)
  {
    sum += readColor(s2, s3);
  }

  return sum / 10;
}

int mapColor(int value, int minVal, int maxVal)
{
  int mapped = map(value, minVal, maxVal, 255, 0);

  return constrain(mapped, 0, 255);
}

/* ---------------- SETUP ---------------- */

void setup()
{
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  /* -------- TCS3200 -------- */

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  /* -------- BME680 -------- */

  if (!bme.begin(0x76))
  {
    if (!bme.begin(0x77))
    {
      Serial.println("BME680_NOT_FOUND");

      while (1);
    }
  }

  bme.setTemperatureOversampling(BME680_OS_8X);

  bme.setHumidityOversampling(BME680_OS_2X);

  bme.setPressureOversampling(BME680_OS_4X);

  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  bme.setGasHeater(320, 150);

  Serial.println("SYSTEM_READY");

  Serial.println("REMOVE_FRUIT");

  Serial.println("BASELINE_COLLECTION_STARTED");

  baselineStart = millis();
}

/* ---------------- LOOP ---------------- */

void loop()
{
  /* =========================================
      RESTART SESSION
  ========================================= */

  if (Serial.available())
  {
    char input = Serial.read();

    if (input == 'r')
    {
      baselineDone = false;

      fruitPlaced = false;

      baselineGas = 0;

      gasDifference = 0;

      baselineStart = millis();

      fruitStart = 0;

      Serial.println("\nNEW_SESSION_STARTED");

      Serial.println("REMOVE_FRUIT");

      Serial.println("BASELINE_COLLECTION_STARTED");
    }

    if (input == '1')
    {
      if (baselineDone && !fruitPlaced)
      {
        fruitPlaced = true;

        fruitStart = millis();

        Serial.println("\nFRUIT_MONITORING_STARTED");
      }
    }
  }

  /* =========================================
      CONTINUOUS BME READING
  ========================================= */

  updateBME();

  /* =========================================
      BASELINE COLLECTION FOR 10 MIN
  ========================================= */

  if (!baselineDone)
  {
    Serial.print("BASELINE,");
    Serial.println(gas);

    if (millis() - baselineStart >= 600000)
    {
      baselineGas = gas;

      baselineDone = true;

      Serial.print("BASELINE_FINAL,");
      Serial.println(baselineGas);

      Serial.println("PLACE_FRUIT_AND_SEND_1");
    }

    delay(2000);

    return;
  }

  /* =========================================
      WAIT FOR USER
  ========================================= */

  if (!fruitPlaced)
  {
    delay(1000);
    return;
  }

  /* =========================================
      FRUIT MONITORING FOR 10 MIN
  ========================================= */

  if (millis() - fruitStart <= 600000)
  {
    int redRaw = averageRead(LOW, LOW);

    int greenRaw = averageRead(HIGH, HIGH);

    int blueRaw = averageRead(LOW, HIGH);

    int R = mapColor(redRaw, redMin, redMax);

    int G = mapColor(greenRaw, greenMin, greenMax);

    int B = mapColor(blueRaw, blueMin, blueMax);

    gasDifference = baselineGas - gas;

    Serial.print(R);
    Serial.print(",");

    Serial.print(G);
    Serial.print(",");

    Serial.print(B);
    Serial.print(",");

    Serial.print(temperature);
    Serial.print(",");

    Serial.print(humidity);
    Serial.print(",");

    Serial.print(pressure);
    Serial.print(",");

    Serial.print(gas);
    Serial.print(",");

    Serial.println(gasDifference);
  }

  else
  {
    Serial.println("FRUIT_MONITORING_COMPLETE");

    Serial.println("SEND_R_FOR_NEW_SESSION");

    fruitPlaced = false;
  }

  delay(2000);
}