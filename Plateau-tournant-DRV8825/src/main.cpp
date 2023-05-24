#include <M5Stack.h>

const int PinDir = GPIO_NUM_13;
const int PinStep = GPIO_NUM_5;
const int PinEnable = GPIO_NUM_17;

float speed = 0;
const float MAXSPEED = 10;

const float SPEED2TIME = 200 * 32;

void loopGUI(void *param)
{

  TFT_eSprite img = TFT_eSprite(&M5.Lcd);

  img.setColorDepth(8);
  img.createSprite(320, 100);

  M5.Lcd.setFreeFont(&FreeSans12pt7b);

  M5.Lcd.drawString("Reset", 40, 210);
  M5.Lcd.drawString("-", 158, 210);
  M5.Lcd.drawString("+", 250, 210);

  img.setFreeFont(&FreeSans24pt7b);
  img.setTextSize(2);
  int a1 = img.textWidth("0.00");
  int a2 = img.textWidth("10.00");
  int b1 = img.textWidth("-0.00");
  int b2 = img.textWidth("-10.00");

  while (true)
  {
    float inc = 0;
    char str[80];

    M5.update();

    if (M5.BtnB.wasPressed() || M5.BtnB.pressedFor(300, 10))
      inc = -0.01;
    if (M5.BtnC.wasPressed() || M5.BtnC.pressedFor(300, 10))
      inc = 0.01;
    if (M5.BtnA.wasPressed())
      speed = 0;

    speed = constrain(speed + inc, -MAXSPEED, MAXSPEED);

    img.fillSprite(BLACK);
    img.setTextColor(TFT_WHITE);
    img.setTextDatum(BL_DATUM);
    img.setFreeFont(&FreeSans24pt7b);
    img.setTextSize(2);
    sprintf(str, "%.2f", speed);

    int a;
    if (speed <= -10)
      a = b2;
    else if (speed < 0)
      a = b1;
    else if (speed < 10)
      a = a1;
    else
      a = a2;

    img.drawString(str, 240 - a, 100);

    img.setTextDatum(BL_DATUM);
    img.setTextSize(1);
    img.drawString("t/s", 245, 92);

    img.pushSprite(0, 60);

    delay(5);
  }
}

void setup()
{
  M5.begin();
  pinMode(PinDir, OUTPUT);
  pinMode(PinStep, OUTPUT);
  pinMode(PinEnable, OUTPUT);

  digitalWrite(PinEnable, LOW);
  xTaskCreatePinnedToCore(loopGUI, "GUI", 4000, NULL, 0, NULL, 0);
}

void loop()
{

  static long t0 = micros();
  long t1 = micros();
  long deltat = t1 - t0;
  t0 = t1;
  static long phase = 0;

  if (speed == 0)
  {
    digitalWrite(PinEnable, HIGH);
  }
  else
  {
    digitalWrite(PinEnable, LOW);
    digitalWrite(PinDir, speed > 0);
  }

  phase += fabs(speed) * 200.0 * 32.0 * (float)deltat;
  // phase += (float)deltat * 10;
  if (phase > 1e6)
  {
    digitalWrite(PinStep, HIGH);
    delayMicroseconds(1);
    digitalWrite(PinStep, LOW);
    phase -= 1e6;
  }
}