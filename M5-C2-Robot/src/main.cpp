#include <M5core2.h>

Button buttonOn(70, 30, 180, 180);

TFT_eSprite img = TFT_eSprite(&M5.Lcd);
/////// Pin Config //////////
int PinMotor1[] = {25, 26, 13, 14};
int PinMotor2[] = {32, 33, 27, 19};

// TaskHandle_t taskGUI;

const int N = 1000;
int TEMP = 2000;

bool motorOn;

void drawPlay(Button &b, ButtonColors bc)
{
    img.fillRoundRect(b.x, b.y, b.w, b.h, 10, DARKGREY);

  if (motorOn)
  {
    img.fillRect(b.x + b.w / 4, b.y + b.h / 4, b.w / 2, b.h / 2, RED);
  }
  else
  {
    img.fillTriangle(b.x + 0.3*b.w , b.y + b.h / 4,
                     b.x + 0.3*b.w , b.y + 3 * b.h / 4,
                     b.x + 3*b.w / 4, b.y + b.h / 2, GREEN);
  }
}

void loopGUI(void *param)
{

  /// Setup

  img.setColorDepth(8);
  img.setTextColor(TFT_WHITE);
  img.createSprite(320, 240);

  ///

  while (true) // never return
  {

    M5.update();

    img.setFreeFont(&FreeSans12pt7b);
    img.fillSprite(BLACK);

    char str[100];
    float vb = M5.Axp.GetBatVoltage();
    float ib = M5.Axp.GetBatCurrent();

    sprintf(str,"Batt: %3d%%  %4.2fV  %6.1fmA  ", (int)((vb - 3.2) * 100), vb, ib);
    img.drawString(str,0,0);

    /*M5.Lcd.setCursor(0, 0);

    float vb = M5.Axp.GetBatVoltage();
    float ib = M5.Axp.GetBatCurrent();

    M5.Lcd.printf("Batterie %3d%%, voltage= %4.2fV  courant=%6.2fmA  ", (int)((vb - 3.2) * 100), vb, ib);

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("35 :  %4d   36:  %4d", analogRead(35), analogRead(36));
*/
    buttonOn.draw();
    img.pushSprite(0, 0);

    ///// Handle buttons ////

    // if (M5.BtnA.wasPressed())
    if (buttonOn.wasPressed())
    {
      motorOn = !motorOn;
    }

    delay(10);
  }
}

void setup()
{
  buttonOn.drawFn = drawPlay;

  M5.begin(true, false, false, false);
  M5.Lcd.print("coucou");

  for (int i = 0; i < 4; i++)
  {
    pinMode(PinMotor1[i], OUTPUT);
    pinMode(PinMotor2[i], OUTPUT);
  }

  pinMode(35, INPUT_PULLDOWN);

  M5.Axp.SetCHGCurrent(4);

  buttonOn.draw();

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
