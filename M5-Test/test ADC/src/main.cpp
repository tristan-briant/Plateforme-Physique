#include <Arduino.h>

void setup()
{
  pinMode(GPIO_NUM_13, OUTPUT);
  Serial.begin(2000000);
}

void loop()
{
  digitalWrite(13,1);
  delay(500);
   digitalWrite(13,0);
  delay(500);
  Serial.println(analogRead(0));
}