#include <M5core2.h>

const int ButtonSize = 180;

Button buttonOn((320 - ButtonSize) / 2, 30, ButtonSize, ButtonSize);

TFT_eSprite imgButOn = TFT_eSprite(&M5.Lcd);
TFT_eSprite imgButOff = TFT_eSprite(&M5.Lcd);
TFT_eSprite banner = TFT_eSprite(&M5.Lcd);

/////// Pin Config //////////
int PinMotor1[] = {25, 26, 13, 14};
int PinMotor2[] = {32, 33, 27, 19};


const int N = 1000;
int TEMP = 2000;

bool motorOn;



void loopGUI(void *param)
{
  u_long t0 = 0, t1;

  /// Setup

  // Button images creation
  imgButOn.setColorDepth(8);
  imgButOn.createSprite(ButtonSize, ButtonSize);

  imgButOff.setColorDepth(8);
  imgButOff.createSprite(ButtonSize, ButtonSize);

  imgButOff.fillRoundRect(0, 0, ButtonSize, ButtonSize, 10, DARKGREY);
  imgButOff.fillRect(ButtonSize / 4, ButtonSize / 4, ButtonSize / 2, ButtonSize / 2, RED);

  imgButOn.fillRoundRect(0, 0, ButtonSize, ButtonSize, 10, DARKGREY);
  imgButOn.fillTriangle(0.3 * ButtonSize, ButtonSize / 4,
                        0.3 * ButtonSize, 3 * ButtonSize / 4,
                        3 * ButtonSize / 4, ButtonSize / 2, GREEN);

  imgButOn.pushSprite((320 - ButtonSize) / 2, 30);

  // Banner image creation
  char str[100];
  banner.setColorDepth(8);
  banner.setTextColor(TFT_WHITE);
  banner.createSprite(320, 20);

  while (true) // never return
  {
    t1 = millis();
    M5.update();

    ///// Handle buttons ////

    if (buttonOn.wasReleased())
    {
      motorOn = !motorOn;
      if (motorOn)
        imgButOff.pushSprite((320 - ButtonSize) / 2, 30);
      else
        imgButOn.pushSprite((320 - ButtonSize) / 2, 30);
    }

    // Redraw Banner

    if (t1 - t0 > 200UL)
    {
      float vb = M5.Axp.GetBatVoltage();
      float ib = M5.Axp.GetBatCurrent();

      sprintf(str, "Batt: %3d%%  %4.2fV  %6.1fmA  ", (int)((vb - 3.2) * 100), vb, ib);

      banner.setFreeFont(&FreeSans12pt7b);
      banner.fillSprite(BLACK);
      banner.drawString(str, 0, 0);

      banner.pushSprite(0, 0);
      t0 = t1;
    }

    delay(1);
  }
}

void setup()
{
  Serial.begin(115200);

  M5.begin(true, false, false, false);
 
  for (int i = 0; i < 4; i++)
  {
    pinMode(PinMotor1[i], OUTPUT);
    pinMode(PinMotor2[i], OUTPUT);
  }

  //pinMode(35, INPUT_PULLDOWN);

  M5.Axp.SetCHGCurrent(4);

  xTaskCreatePinnedToCore(loopGUI, "Task GUI", 4000, NULL, 0, NULL, 0);
}

void loop()
{

  unsigned int c1, c2 = 0;
  int *p1 = PinMotor1;
  int *p2 = PinMotor2;

  int direction1 = 1;
  int direction2 = -1;

  while (true)
  { // never return

    if (motorOn)
    {
      digitalWrite(p1[c1], HIGH);
      digitalWrite(p1[(c1 + 1) % 4], HIGH);
    }
    if (motorOn)
    {
      digitalWrite(p2[c2], HIGH);
      digitalWrite(p2[(c2 + 1) % 4], HIGH);
    }
    delayMicroseconds(TEMP);

    if (direction1 == 1)
      digitalWrite(p1[c1], LOW);
    if (direction1 == -1)
      digitalWrite(p1[(c1 + 1) % 4], LOW);

    if (direction2 == 1)
      digitalWrite(p2[c2], LOW);
    if (direction2 == -1)
      digitalWrite(p2[(c2 + 1) % 4], LOW);

    c1 = (c1 + direction1) % 4;
    c2 = (c2 + direction2) % 4;
  }
}
