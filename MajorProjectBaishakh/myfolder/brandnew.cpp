
#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/* =====================================================
   WIFI
===================================================== */

const char* ssid = "nrb116_fpkhr";
const char* password = "Acharya116@";

const char* server = "http://192.168.1.99:5000/predict";

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
   SYSTEM STATES
===================================================== */

enum SystemState
{
  BASELINE,
  WAIT_FRUIT,
  STABILIZING,
  MONITORING,
  COMPLETE
};

SystemState currentState = BASELINE;

/* =====================================================
   TIMING VARIABLES
===================================================== */

unsigned long baselineStart = 0;
unsigned long stabilizationStart = 0;
unsigned long monitoringStart = 0;

unsigned long lastBMERead = 0;
unsigned long lastColorRead = 0;
unsigned long lastHTTPSend = 0;
unsigned long lastPrint = 0;

/* =====================================================
   INTERVALS
===================================================== */

const unsigned long BME_INTERVAL = 1000;
const unsigned long COLOR_INTERVAL = 5000;
const unsigned long HTTP_INTERVAL = 10000;
const unsigned long PRINT_INTERVAL = 2000;

const unsigned long BASELINE_DURATION = 600000;
const unsigned long STABILIZATION_DURATION = 60000;
const unsigned long MONITORING_DURATION = 600000;

/* =====================================================
   SENSOR VARIABLES
===================================================== */

float temperature = 0;
float humidity = 0;
float pressure = 0;
float gas = 0;

float smoothedGas = 0;

float baselineGas = 0;
float baselineSum = 0;
int baselineCount = 0;

float gasDifference = 0;
float vocPercent = 0;

/* =====================================================
   COLOR VARIABLES
===================================================== */

int R = 0;
int G = 0;
int B = 0;

/* =====================================================
   DATA STORAGE
===================================================== */

String storedData = "";

/* =====================================================
   WIFI CONNECT
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

  Serial.println("\nWiFi Connected");
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

    if (smoothedGas == 0)
    {
      smoothedGas = gas;
    }

    smoothedGas = (0.9 * smoothedGas) + (0.1 * gas);
  }
}

/* =====================================================
   TCS3200 FUNCTIONS
===================================================== */

int readColor(bool s2, bool s3)
{
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);

  delay(5);

  return pulseIn(sensorOut, LOW, 100000);
}

int averageRead(bool s2, bool s3)
{
  long sum = 0;

  for (int i = 0; i < 5; i++)
  {
    sum += readColor(s2, s3);
  }

  return sum / 5;
}

int mapColor(int value, int minVal, int maxVal)
{
  int mapped = map(value, minVal, maxVal, 255, 0);

  return constrain(mapped, 0, 255);
}

void updateColor()
{
  int redRaw = averageRead(LOW, LOW);

  int greenRaw = averageRead(HIGH, HIGH);

  int blueRaw = averageRead(LOW, HIGH);

  R = mapColor(redRaw, redMin, redMax);

  G = mapColor(greenRaw, greenMin, greenMax);

  B = mapColor(blueRaw, blueMin, blueMax);
}

/* =====================================================
   SEND TO SERVER
===================================================== */

void sendToServer()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
    return;
  }

  HTTPClient http;

  http.setTimeout(2000);

  http.begin(server);

  http.addHeader("Content-Type", "application/json");

  String json = "{";

  json += "\"Red\":" + String(R) + ",";
  json += "\"Green\":" + String(G) + ",";
  json += "\"Blue\":" + String(B) + ",";

  json += "\"Temperature\":" + String(temperature, 2) + ",";
  json += "\"Humidity\":" + String(humidity, 2) + ",";
  json += "\"Pressure\":" + String(pressure, 2) + ",";

  json += "\"GasResistance\":" + String(smoothedGas, 2) + ",";

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

/* =====================================================
   SAVE DATA
===================================================== */

void saveData()
{
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

  row += String(smoothedGas, 2);
  row += ",";

  row += String(gasDifference, 2);
  row += ",";

  row += String(vocPercent, 2);

  storedData += row + "\n";
}

/* =====================================================
   PRINT SENSOR DATA
===================================================== */

void printData()
{
  Serial.println("\n========================");

  Serial.print("R: ");
  Serial.println(R);

  Serial.print("G: ");
  Serial.println(G);

  Serial.print("B: ");
  Serial.println(B);

  Serial.print("Temperature: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Pressure: ");
  Serial.println(pressure);

  Serial.print("Raw Gas: ");
  Serial.println(gas);

  Serial.print("Smoothed Gas: ");
  Serial.println(smoothedGas);

  Serial.print("Difference: ");
  Serial.println(gasDifference);

  Serial.print("VOC %: ");
  Serial.println(vocPercent);
}

/* =====================================================
   RESET SESSION
===================================================== */

void resetSession()
{
  baselineGas = 0;
  baselineSum = 0;
  baselineCount = 0;

  gasDifference = 0;
  vocPercent = 0;

  storedData = "";

  baselineStart = millis();

  currentState = BASELINE;

  Serial.println("\nNEW SESSION STARTED");
  Serial.println("REMOVE FRUIT");
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

      while (1);
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
     ALWAYS RUN BME680
  ===================================================== */

  if (millis() - lastBMERead >= BME_INTERVAL)
  {
    lastBMERead = millis();

    updateBME();
  }

  /* =====================================================
     SERIAL COMMANDS
  ===================================================== */

  if (Serial.available())
  {
    char input = Serial.read();

    if (input == 'r')
    {
      resetSession();
    }

    if (input == '1')
    {
      if (currentState == WAIT_FRUIT)
      {
        stabilizationStart = millis();

        currentState = STABILIZING;

        Serial.println("\nFRUIT DETECTED");
        Serial.println("STABILIZING FOR 60 SECONDS");
      }
    }
  }

  /* =====================================================
     BASELINE PHASE
  ===================================================== */

  if (currentState == BASELINE)
  {
    baselineSum += smoothedGas;

    baselineCount++;

    if (millis() - lastPrint >= PRINT_INTERVAL)
    {
      lastPrint = millis();

      Serial.print("BASELINE GAS: ");
      Serial.println(smoothedGas);
    }

    if (millis() - baselineStart >= BASELINE_DURATION)
    {
      baselineGas = baselineSum / baselineCount;

      currentState = WAIT_FRUIT;

      Serial.println("\nBASELINE COMPLETE");

      Serial.print("BASELINE AVERAGE: ");
      Serial.println(baselineGas);

      Serial.println("PLACE FRUIT AND SEND 1");
    }
  }

  /* =====================================================
     STABILIZATION PHASE
  ===================================================== */

  if (currentState == STABILIZING)
  {
    if (millis() - lastPrint >= PRINT_INTERVAL)
    {
      lastPrint = millis();

      Serial.print("STABILIZING GAS: ");
      Serial.println(smoothedGas);
    }

    if (millis() - stabilizationStart >= STABILIZATION_DURATION)
    {
      monitoringStart = millis();

      currentState = MONITORING;

      Serial.println("\nMONITORING STARTED");
    }
  }

  /* =====================================================
     MONITORING PHASE
  ===================================================== */

  if (currentState == MONITORING)
  {
    gasDifference = baselineGas - smoothedGas;

    if (baselineGas != 0)
    {
      vocPercent = gasDifference / baselineGas;
    }
    else
    {
      vocPercent = 0;
    }

    /* ---------------- COLOR UPDATE ---------------- */

    if (millis() - lastColorRead >= COLOR_INTERVAL)
    {
      lastColorRead = millis();

      updateColor();

      saveData();

      printData();
    }

    /* ---------------- HTTP SEND ---------------- */

    if (millis() - lastHTTPSend >= HTTP_INTERVAL)
    {
      lastHTTPSend = millis();

      sendToServer();
    }

    /* ---------------- MONITORING COMPLETE ---------------- */

    if (millis() - monitoringStart >= MONITORING_DURATION)
    {
      currentState = COMPLETE;

      Serial.println("\nMONITORING COMPLETE");

      Serial.println("\nSTORED DATA:");
      Serial.println(storedData);

      Serial.println("SEND r FOR NEW SESSION");
    }
  }
}
