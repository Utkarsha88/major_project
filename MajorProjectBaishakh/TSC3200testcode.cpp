#include <Arduino.h>

#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

int readColor(bool s2, bool s3) {

  digitalWrite(S2, s2);
  digitalWrite(S3, s3);

  delay(50);

  int frequency = pulseIn(sensorOut, LOW);

  return frequency;
}

int averageColor(bool s2, bool s3) {

  long sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += readColor(s2, s3);
  }

  return sum / 10;
}

void setup() {

  Serial.begin(115200);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // 20% scaling
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("Stable TCS3200 Test");
}

void loop() {

  int red = averageColor(LOW, LOW);
  int green = averageColor(HIGH, HIGH);
  int blue = averageColor(LOW, HIGH);

  Serial.print("R = ");
  Serial.print(red);

  Serial.print("  G = ");
  Serial.print(green);

  Serial.print("  B = ");
  Serial.println(blue);

  delay(1000);
}