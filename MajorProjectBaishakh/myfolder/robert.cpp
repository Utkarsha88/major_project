#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/* =====================================================
   WIFI
===================================================== */

const char *ssid = "nrb116_fpkhr";
const char *password = "Acharya116@";

const char *server =
    "http://192.168.1.88:5000/api/v1/sensor-data";
const char *apiKey = "major-project-secret-key873468734r";

/* =====================================================
   BME680
===================================================== */

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BME680 bme;

/* =====================================================
   TCS3200
===================================================== */

#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

/* =====================================================
   COLOR CALIBRATION
===================================================== */

int redMin = 60;
int redMax = 400;

int greenMin = 65;
int greenMax = 450;

int blueMin = 50;
int blueMax = 350;

/* =====================================================
   GLOBAL VARIABLES
===================================================== */

float temperature = 0;
float humidity = 0;
float pressure = 0;
float gas = 0;

float baselineGas = 0;
float gasDifference = 0;
float vocPercent = 0;
/* =====================================================
   DATA STORAGE
===================================================== */

String storedData = "";

/* =====================================================
   CONTROL VARIABLES
===================================================== */

bool baselineDone = false;
bool fruitPlaced = false;

unsigned long baselineStart = 0;
unsigned long fruitStart = 0;

/* =====================================================
   WIFI CONNECT FUNCTION
===================================================== */

void connectWiFi()
{
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

/* =====================================================
   BME680 UPDATE
===================================================== */

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

/* =====================================================
   TCS3200 FUNCTIONS
===================================================== */

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

/* =====================================================
   SEND DATA TO FLASK
===================================================== */

void sendToServer(
    int R,
    int G,
    int B)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    http.begin(server);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", apiKey);

    String json = "{";

    json += "\"Red\":" + String(R) + ",";
    json += "\"Green\":" + String(G) + ",";
    json += "\"Blue\":" + String(B) + ",";

    json += "\"Temperature\":" + String(temperature, 2) + ",";
    json += "\"Humidity\":" + String(humidity, 2) + ",";
    json += "\"Pressure\":" + String(pressure, 2) + ",";

    json += "\"Gas resistance in (Kohm)\":" + String(gas, 2) + ",";

    json += "\"Difference\":" + String(gasDifference, 2) + ",";

    json += "\"VOC_percent\":" + String(vocPercent, 2);

    json += "}";

    Serial.println("\nSENDING JSON:");
    Serial.println(json);

    int httpCode = http.POST(json);

    Serial.print("HTTP CODE: ");
    Serial.println(httpCode);

    if (httpCode > 0)
    {
      String response = http.getString();

      Serial.println("SERVER RESPONSE:");

      Serial.println(response);
    }

    else
    {
      Serial.println("HTTP REQUEST FAILED");
    }

    http.end();
  }

  else
  {
    Serial.println("WIFI DISCONNECTED");

    connectWiFi();
  }
}

/* =====================================================
   SETUP
===================================================== */

void setup()
{
  Serial.begin(115200);

  /* ---------------- WIFI ---------------- */

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  connectWiFi();

  /* ---------------- I2C ---------------- */

  Wire.begin(SDA_PIN, SCL_PIN);

  /* ---------------- TCS3200 ---------------- */

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  /* ---------------- BME680 ---------------- */

  if (!bme.begin(0x76))
  {
    if (!bme.begin(0x77))
    {
      Serial.println("BME680 NOT FOUND");

      while (1)
        ;
    }
  }

  bme.setTemperatureOversampling(BME680_OS_8X);

  bme.setHumidityOversampling(BME680_OS_2X);

  bme.setPressureOversampling(BME680_OS_4X);

  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  bme.setGasHeater(320, 150);

  Serial.println("\nSYSTEM READY");

  Serial.println("REMOVE FRUIT");

  Serial.println("BASELINE COLLECTION STARTED");

  baselineStart = millis();
}

/* =====================================================
   LOOP
===================================================== */

void loop()
{
  /* =====================================================
     SERIAL COMMANDS
  ===================================================== */

  if (Serial.available())
  {
    char input = Serial.read();

    /* ------------ RESET SESSION ------------ */

    if (input == 'r')
    {
      storedData = "";
      baselineDone = false;

      fruitPlaced = false;

      baselineGas = 0;

      gasDifference = 0;

      vocPercent = 0;

      baselineStart = millis();

      fruitStart = 0;

      Serial.println("\nNEW SESSION STARTED");

      Serial.println("REMOVE FRUIT");

      Serial.println("BASELINE COLLECTION STARTED");
    }

    /* ------------ START FRUIT ------------ */

    if (input == '1')
    {
      if (baselineDone && !fruitPlaced)
      {
        fruitPlaced = true;

        fruitStart = millis();

        Serial.println("\nFRUIT MONITORING STARTED");
      }
    }
  }

  /* =====================================================
     UPDATE BME680
  ===================================================== */

  updateBME();

  /* =====================================================
     BASELINE COLLECTION
  ===================================================== */

  if (!baselineDone)
  {
    Serial.print("BASELINE GAS: ");
    Serial.println(gas);

    if (millis() - baselineStart >= 600000)
    {
      baselineGas = gas;

      baselineDone = true;

      Serial.print("\nBASELINE FINAL: ");
      Serial.println(baselineGas);

      Serial.println("PLACE FRUIT AND SEND 1");
    }

    delay(2000);

    return;
  }

  /* =====================================================
     WAIT FOR FRUIT
  ===================================================== */

  if (!fruitPlaced)
  {
    delay(1000);

    return;
  }

  /* =====================================================
     FRUIT MONITORING
  ===================================================== */

  if (millis() - fruitStart <= 600000)
  {
    int redRaw = averageRead(LOW, LOW);

    int greenRaw = averageRead(HIGH, HIGH);

    int blueRaw = averageRead(LOW, HIGH);

    int R = mapColor(redRaw, redMin, redMax);

    int G = mapColor(greenRaw, greenMin, greenMax);

    int B = mapColor(blueRaw, blueMin, blueMax);

    gasDifference = baselineGas - gas;

    if (baselineGas != 0)
    {
      vocPercent = gasDifference / baselineGas;
    }

    else
    {
      vocPercent = 0;
    }
    delay(100);

    /* ------------ SERIAL DEBUG ------------ */

    Serial.println("\n========================");

    Serial.print("R: ");
    Serial.println(R);

    Serial.print("G: ");
    Serial.println(G);

    Serial.print("B: ");
    Serial.println(B);

    Serial.print("Temp: ");
    Serial.println(temperature);

    Serial.print("Humidity: ");
    Serial.println(humidity);

    Serial.print("Pressure: ");
    Serial.println(pressure);

    Serial.print("Gas: ");
    Serial.println(gas);

    Serial.print("Difference: ");
    Serial.println(gasDifference);

    Serial.print("VOC %: ");
    Serial.println(vocPercent);

    String row = "";

    row += String(R);
    row += ",";

    row += String(G);
    row += ",";

    row += String(B);
    row += ",";

    row += String(temperature, 2);
    row += ",";

    row += String(humidity, 2);
    row += ",";

    row += String(pressure, 2);
    row += ",";

    row += String(gas, 2);
    row += ",";

    row += String(gasDifference, 2);

    storedData += row + "\n";

    /* ------------ SEND TO SERVER ------------ */

    sendToServer(R, G, B);
  }

  else
  {
    Serial.println("\nFRUIT MONITORING COMPLETE");
    Serial.println(storedData);

    Serial.println("SEND r FOR NEW SESSION");

    fruitPlaced = false;
  }

  delay(5000);
}