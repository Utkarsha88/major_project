
#include<Arduino.h>
#define S0 4
#define S1 5
#define S2 18
#define S3 19
#define OUT_PIN 21
// #define LED_PIN 5


#define MQ3_PIN 34

int readColor(int s2State, int s3State) {
  digitalWrite(S2, s2State);
  digitalWrite(S3, s3State);
  delay(20);
  return pulseIn(OUT_PIN, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  // pinMode(LED_PIN, OUTPUT);

  // Frequency scaling 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  // digitalWrite(LED_PIN, HIGH); 

  Serial.println("Banana Test Started...");
}

void loop() {

  int red   = readColor(LOW, LOW);
  int green = readColor(HIGH, HIGH);
  int blue  = readColor(LOW, HIGH);

  Serial.print("R:");
  Serial.print(red);
  Serial.print(" G:");
  Serial.print(green);
  Serial.print(" B:");
  Serial.println(blue);


  String ripeness;

  if (green > red && green < blue) {
    ripeness = "RIPE YELLOW";
  }
  else if (red > green) {
    ripeness = "UNRIPE GREEN";
  }
  else if (blue > red && blue > green) {
    ripeness = "BROWN/OVERRIPE";
  }
  else ripeness = "UNKNOWN";

  // ---- GAS SENSOR READ ----
  int gasValue = analogRead(MQ3_PIN);

  Serial.print("Gas Level: ");
  Serial.println(gasValue);

  String chemicalStatus;

  if (gasValue > 2000)
    chemicalStatus = "HIGH — Possible chemical ripening!";
  else if (gasValue > 1300)
    chemicalStatus = "Moderate — Possible ethylene release";
  else
    chemicalStatus = "Normal";

  // ---- FINAL OUTPUT ----
  Serial.print("Ripeness: ");
  Serial.print(ripeness);
  Serial.print(" | Chemical: ");
  Serial.println(chemicalStatus);

  delay(1000);
}
