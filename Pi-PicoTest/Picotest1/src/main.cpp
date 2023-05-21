#include <Arduino.h>

void setup()
{
  pinMode(25, OUTPUT);
}

void loop()
{
  digitalWrite(25, HIGH);
  delay(100);
  digitalWrite(25, LOW);
  delay(500);
}