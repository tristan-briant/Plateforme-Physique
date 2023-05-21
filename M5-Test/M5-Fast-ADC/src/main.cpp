#include <Arduino.h>
#include "driver/adc.h"


void setup()
{
  Serial.begin(2000000);
  pinMode(25, OUTPUT);
  digitalWrite(25, 0);
  adc_set_clk_div(1);
  Serial.println(xPortGetCoreID());
}

void loop()
{
  Serial.println(xPortGetCoreID());

  int a;
  long t = micros();
  //a = adc1_get_raw(adc1_channel_t::ADC1_CHANNEL_7);
 // a = analogRead(35);
  long tt = micros() - t;

  Serial.printf("%5d   %d\n", tt, a);
  delay(1000);
}