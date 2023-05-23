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

  while (true)
  {
    float inc = 0;
    char str[80];

    M5.update();

    if (M5.BtnB.wasPressed() || M5.BtnB.pressedFor(300, 25))
      inc = -0.02;
    if (M5.BtnC.wasPressed() || M5.BtnC.pressedFor(300, 25))
      inc = 0.02;
    if (M5.BtnA.wasPressed())
      speed = 0;

    speed = constrain(speed + inc, -MAXSPEED, MAXSPEED);

    img.fillSprite(BLACK);
    img.setTextColor(TFT_WHITE);
    img.setTextDatum(BL_DATUM);
    img.setFreeFont(&FreeSans24pt7b);

    img.setTextSize(2);
    sprintf(str, "%.2f", speed);
    int a = img.textWidth(str);
    img.drawString(str, 240 - a, 100);
    img.setTextSize(1);
    img.drawString("t/s", 245, 92);

    img.pushSprite(0, 60);

    delay(10);
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
  static long phase;

  if (speed == 0)
  {
    digitalWrite(PinEnable, HIGH);
    //phase = t;
  }
  else
  {
    digitalWrite(PinEnable, LOW);
    digitalWrite(PinDir, speed > 0);

    long tt = fabs(speed) * SPEED2TIME * deltat;

    if (t > phase + tt)
    {
      digitalWrite(PinStep, HIGH);
      delayMicroseconds(1);
      digitalWrite(PinStep, LOW);
      phase += tt;
    }
  }
}