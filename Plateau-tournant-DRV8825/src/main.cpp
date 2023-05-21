#include <M5Stack.h>

const int PinDir = GPIO_NUM_13;
const int PinStep = GPIO_NUM_5;
const int PinEnable = GPIO_NUM_17;

float speed = 0;

void setup()
{
  M5.begin();
  pinMode(PinDir, OUTPUT);
  pinMode(PinStep, OUTPUT);
  pinMode(PinEnable, OUTPUT);

  digitalWrite(PinEnable, LOW);
}

void loop()
{
  M5.update();

  if (M5.BtnB.wasPressed())
    speed = 1 - speed;

    M5.Lcd.setCursor(120,120);
    M5.Lcd.print(speed);

   if (speed == 0)
     digitalWrite(PinEnable, HIGH);
   else
     digitalWrite(PinEnable, LOW);

  digitalWrite(PinStep, LOW);
  delayMicroseconds(10000);
  digitalWrite(PinStep, HIGH);
  delayMicroseconds(10000);
}