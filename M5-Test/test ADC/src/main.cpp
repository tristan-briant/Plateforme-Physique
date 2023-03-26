#include <Arduino.h>

void setup()
{
  pinMode(GPIO_NUM_0, INPUT);
  Serial.begin(2000000);
}

void loop()
{
  Serial.println(analogRead(0));
}