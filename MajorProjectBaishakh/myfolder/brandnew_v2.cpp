#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/* =====================================================
   WIFI
===================================================== */

const char *ssid = "nrb116_fpkhr";
const char *password = "Acharya116@";

const char *server =
    "https://fruit-pulse-backend.onrender.com/api/v1/sensor-data";
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
#define START_SWITCH 18
#define S0 33
#define S1 32
#define S2 23
#define S3 19
#define sensorOut 35

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
  IDLE,
  BASELINE,
  WAIT_FRUIT,
  STABILIZING,
  MONITORING,
  COMPLETE
};

SystemState currentState = IDLE;

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
unsigned long lastBaselineSample = 0;
const unsigned long BASELINE_SAMPLE_INTERVAL = 1000; // 1 second
const unsigned long WAIT_FRUIT_DURATION = 15000;     // 15 sec to place fruit
const unsigned long COMPLETE_WAIT_DURATION = 10000;  // 10 sec before reset

unsigned long waitFruitStart = 0;
unsigned long completeStart = 0;

/* =====================================================
   INTERVALS
===================================================== */

const unsigned long BME_INTERVAL = 1000;
const unsigned long COLOR_INTERVAL = 5000;
const unsigned long HTTP_INTERVAL = 10000;
const unsigned long PRINT_INTERVAL = 2000;

const unsigned long BASELINE_DURATION = 300000;
const unsigned long STABILIZATION_DURATION = 30000;
const unsigned long MONITORING_DURATION = 600000; // 10 minutes
bool systemEnabled = false;

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

String storedData = "";

/* =====================================================
   🆕 ML FEATURES
===================================================== */

float previousGas = 0;
float gasRateOfChange = 0;

float gasBuffer[10] = {0};
int gasIndex = 0;
float stabilityIndex = 0;

/* =====================================================
   COLOR VARIABLES
===================================================== */

int R = 0;
int G = 0;
int B = 0;

/* =====================================================
   DATA STORAGE
===================================================== */

String jsonArray = "";
bool hasSamples = false;
int sampleCount = 0;
unsigned long lastSampleTime = 0;
const unsigned long SAMPLE_INTERVAL = 5000; // 5 seconds

String formatCurrentReadingJson()
{
  String json = "{";

  json += "\"Red\":" + String(R) + ",";
  json += "\"Green\":" + String(G) + ",";
  json += "\"Blue\":" + String(B) + ",";

  json += "\"Temperature\":" + String(temperature, 2) + ",";
  json += "\"Humidity\":" + String(humidity, 2) + ",";
  json += "\"Pressure\":" + String(pressure, 2) + ",";

  json += "\"GasResistance\":" + String(smoothedGas, 2) + ",";
  json += "\"Difference\":" + String(gasDifference, 2) + ",";
  json += "\"VOC_percent\":" + String(vocPercent, 2) + ",";

  // json += "\"GasRate\":" + String(gasRateOfChange, 2) + ",";
  // json += "\"Stability\":" + String(stabilityIndex, 2);

  json += "}";
  return json;
}

void addSampleToJsonArray()
{
  String sample = formatCurrentReadingJson();
  if (hasSamples)
  {
    jsonArray += ",";
  }
  jsonArray += sample;
  hasSamples = true;
}



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
      smoothedGas = gas;

    smoothedGas = (0.9 * smoothedGas) + (0.1 * gas);

    /* Calculate gas rate of change when there's a new reading */
    gasRateOfChange = smoothedGas - previousGas;
    previousGas = smoothedGas;

    

    

    
  }
}

/* =====================================================
   COLOR SENSOR
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
    sum += readColor(s2, s3);
  return sum / 5;
}

int mapColor(int value, int minVal, int maxVal)
{
  return constrain(map(value, minVal, maxVal, 255, 0), 0, 255);
}

void updateColor()
{
  R = mapColor(averageRead(LOW, LOW), redMin, redMax);
  G = mapColor(averageRead(HIGH, HIGH), greenMin, greenMax);
  B = mapColor(averageRead(LOW, HIGH), blueMin, blueMax);
}

/* =====================================================
   SEND TO SERVER
===================================================== */
void sendToServer()
{
  if (!hasSamples)
  {
    Serial.println("No samples to send.");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected!");
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Cannot send, WiFi still disconnected.");
      return;
    }
  }

  HTTPClient http;

  http.begin(server);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", apiKey);

  String payload = "[" + jsonArray + "]";

  Serial.println("Sending payload:");
  Serial.println(payload);

  int httpCode = http.POST(payload);

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
   PRINT DATA
===================================================== */

void printData()
{
  Serial.println("\n========================");
  Serial.println("R: " + String(R));
  Serial.println("G: " + String(G));
  Serial.println("B: " + String(B));
  Serial.println("Temp: " + String(temperature));
  Serial.println("Humidity: " + String(humidity));
  Serial.println("Pressure: " + String(pressure));
  Serial.println("Raw Gas: " + String(gas, 2));
  Serial.println("Smoothed Gas: " + String(smoothedGas, 2));
  Serial.println("Previous Gas: " + String(previousGas, 2));
  Serial.println("Diff: " + String(gasDifference, 2));
  Serial.println("VOC%: " + String(vocPercent, 2));
  
 
  Serial.println();
  Serial.println("========================\n");
}

/* =====================================================
   RESET
===================================================== */

void resetSession()
{
  baselineGas = 0;
  baselineSum = 0;
  baselineCount = 0;

  gasDifference = 0;
  vocPercent = 0;

  smoothedGas = 0;
  gas = 0;

  previousGas = 0;
  gasIndex = 0;
  stabilityIndex = 0;

  jsonArray = "";
  hasSamples = false;
  sampleCount = 0;
  lastSampleTime = 0;

  waitFruitStart = 0;
  stabilizationStart = 0;
  monitoringStart = 0;
  completeStart = 0;
  storedData = "";

  lastBaselineSample = 0;
  lastPrint = 0;

  Serial.println("SESSION RESET DONE");
}

/* =====================================================
   SETUP (unchanged parts omitted for brevity)
===================================================== */

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  connectWiFi();
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(START_SWITCH, INPUT_PULLUP);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  if (!bme.begin(0x76))
    if (!bme.begin(0x77))
    {
      Serial.println("BME680 NOT FOUND");
      while (1)
        ;
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
     ALWAYS RUN COLOR SENSOR
  ===================================================== */

  if (millis() - lastColorRead >= COLOR_INTERVAL)
  {
    lastColorRead = millis();

    updateColor();
  }

  /* =====================================================
     BUTTON PRESS TO START SYSTEM
  ===================================================== */

  static bool lastButtonState = HIGH;

  bool currentButtonState = digitalRead(START_SWITCH);

  if (lastButtonState == HIGH && currentButtonState == LOW)
  {
    if (currentState == IDLE)
    {
      Serial.println("\nBUTTON PRESSED");
      Serial.println("STARTING NEW SESSION");

      resetSession();

      currentState = BASELINE;

      baselineStart = millis();

      Serial.println("REMOVE FRUIT");
      Serial.println("BASELINE COLLECTION STARTED");
    }
  }

  lastButtonState = currentButtonState;

  /* =====================================================
     IDLE STATE
  ===================================================== */

  if (currentState == IDLE)
  {
    return;
  }

  /* =====================================================
     BASELINE PHASE
  ===================================================== */

  if (currentState == BASELINE)
  {
    if (millis() - lastBaselineSample >= BASELINE_SAMPLE_INTERVAL)
    {
      lastBaselineSample = millis();

      baselineSum += smoothedGas;
      baselineCount++;

      Serial.print("BASELINE GAS: ");
      Serial.println(smoothedGas);
    }

    if (millis() - baselineStart >= BASELINE_DURATION)
    {
      baselineGas = baselineSum / baselineCount;

      currentState = WAIT_FRUIT;

      waitFruitStart = millis();

      Serial.println("\nBASELINE COMPLETE");
      Serial.print("BASELINE AVERAGE: ");
      Serial.println(baselineGas);

      Serial.println("PLACE FRUIT NOW");
      Serial.println("WAITING 15 SECONDS...");
    }
  }

  /* =====================================================
     WAIT FRUIT
  ===================================================== */

  if (currentState == WAIT_FRUIT)
  {
    if (millis() - waitFruitStart >= WAIT_FRUIT_DURATION)
    {
      stabilizationStart = millis();

      currentState = STABILIZING;

      Serial.println("\nSTABILIZATION STARTED");
    }
  }

  /* =====================================================
     STABILIZATION
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
     MONITORING
  ===================================================== */

  if (currentState == MONITORING)
  {
    gasDifference = baselineGas - smoothedGas;

    vocPercent = (baselineGas != 0)
                     ? gasDifference / baselineGas
                     : 0;

    if (millis() - lastPrint >= PRINT_INTERVAL)
    {
      lastPrint = millis();

      printData();

      saveData();
    }

    if (millis() - lastSampleTime >= SAMPLE_INTERVAL)
    {
      lastSampleTime = millis();

      addSampleToJsonArray();

      sampleCount++;

      Serial.print("Sample added, total count: ");
      Serial.println(sampleCount);
    }

    if (millis() - monitoringStart >= MONITORING_DURATION)
    {
      currentState = COMPLETE;

      completeStart = millis();

      Serial.println("\nMONITORING COMPLETE");

      sendToServer();

      Serial.print("Total samples sent: ");
      Serial.println(sampleCount);
      Serial.println("\nSTORED DATA:");
      Serial.println(storedData);

      Serial.println("PROCESS COMPLETE");
    }
  }

  /* =====================================================
     COMPLETE STATE
  ===================================================== */

  if (currentState == COMPLETE)
  {
    if (millis() - completeStart >= COMPLETE_WAIT_DURATION)
    {
      Serial.println("\nWAITING FOR BUTTON PRESS...");
      
      currentState = IDLE;
    }
  }
}