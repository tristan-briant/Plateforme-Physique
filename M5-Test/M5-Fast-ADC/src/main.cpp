#include <Arduino.h>
#include "driver/adc.h"
#include "driver/dac.h"


void blink(void *paral)
{
  pinMode(GPIO_NUM_2,OUTPUT);
  while (true)
  {
    digitalWrite(2,HIGH);
    delayMicroseconds(12);
    digitalWrite(2,LOW);
    delayMicroseconds(12);
    
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(25, OUTPUT);
  digitalWrite(25, 0);
  adc_set_clk_div(1);
  Serial.println(xPortGetCoreID());
  xTaskCreatePinnedToCore(blink,NULL,4000,NULL,1,NULL,0);// priority 1 sur core 0 => reset
  
}

void loop()
{
  // Serial.println(xPortGetCoreID()); //=1
static int c;
c++;
  int a;
  long t = micros();
  //dacWrite(26,127.0*(1+sin(((float)c*0.1)))); //sin(255*(0.5+(float)c*0.1))
  dac_output_voltage(DAC_CHANNEL_2,127.0*(1+sin(((float)c*0.1))));
  dac_output_voltage(DAC_CHANNEL_2,127.0*(1+sin(((float)c*0.1))));
  dac_output_voltage(DAC_CHANNEL_2,127.0*(1+sin(((float)c*0.1))));
  dac_output_voltage(DAC_CHANNEL_2,127.0*(1+sin(((float)c*0.1))));
  
  // a = analogRead(35);
  long tt = micros() - t;
a = adc1_get_raw(adc1_channel_t::ADC1_CHANNEL_7);
  //Serial.println(xPortGetCoreID());

  Serial.printf("%5d   %d\n", tt, a);
  delay(100);
}