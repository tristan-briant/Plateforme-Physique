#include <M5core2.h>

const int ButtonSize = 180;

Button buttonOn((320 - ButtonSize) / 2, 30, ButtonSize, ButtonSize);

TFT_eSprite imgButOn = TFT_eSprite(&M5.Lcd);
TFT_eSprite imgButOff = TFT_eSprite(&M5.Lcd);
TFT_eSprite imgButTest = TFT_eSprite(&M5.Lcd);
TFT_eSprite banner = TFT_eSprite(&M5.Lcd);

/////// Pin Config //////////
int PinMotor1[] = {25, 26, 13, 14};
int PinMotor2[] = {32, 33, 27, 19};

int PinRightEye = 36;
int PinLeftEye = 35;

const int N = 1000;
int TEMP = 1800;

bool motorLeftOn;
bool motorRightOn;

bool LeftInput, RightInput;
bool falte;

enum RunMode
{
  Idle,
  Run,
  Test,
  Move
};

RunMode mode = RunMode::Idle;

void loopGUI(void *param)
{
  u_long t0 = 0, t1;

  /// Setup

  // Button images creation
  imgButOn.setColorDepth(8);
  imgButOn.createSprite(ButtonSize, ButtonSize);

  imgButOff.setColorDepth(8);
  imgButOff.createSprite(ButtonSize, ButtonSize);

  imgButTest.setColorDepth(8);
  imgButTest.createSprite(ButtonSize, ButtonSize);

  imgButOff.fillRoundRect(0, 0, ButtonSize, ButtonSize, 10, DARKGREY);
  imgButOff.fillRect(ButtonSize / 4, ButtonSize / 4, ButtonSize / 2, ButtonSize / 2, RED);

  imgButOn.fillRoundRect(0, 0, ButtonSize, ButtonSize, 10, DARKGREY);
  imgButOn.fillTriangle(0.3 * ButtonSize, ButtonSize / 4,
                        0.3 * ButtonSize, 3 * ButtonSize / 4,
                        3 * ButtonSize / 4, ButtonSize / 2, GREEN);

  imgButTest.fillRoundRect(0, 0, ButtonSize, ButtonSize, 10, DARKGREY);
  imgButTest.fillTriangle(0.3 * ButtonSize, ButtonSize / 4,
                          0.3 * ButtonSize, 3 * ButtonSize / 4,
                          3 * ButtonSize / 4, ButtonSize / 2, BLUE);

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
    bool redraw = false;

    ///// Handle buttons ////

    if (buttonOn.pressedFor(1000))
    {
      mode = RunMode::Move;
      redraw = true;
    }

    if (buttonOn.wasPressed())
    {
      if (mode != RunMode::Idle)
        mode = RunMode::Idle;
      else
        mode = RunMode::Run;
      redraw = true;
    }

    if(M5.BtnB.isPressed())
      M5.Axp.PowerOff();

      if(M5.BtnA.isPressed()){
        mode = RunMode::Test;
        motorLeftOn = true;
        motorRightOn = false;
        redraw = true;
      }
      if(M5.BtnC.isPressed()){
        mode = RunMode::Test;
        motorLeftOn = false;
        motorRightOn = true;
        redraw = true;
      }

      if(M5.BtnA.wasReleased() || M5.BtnC.wasReleased()){
        mode = RunMode::Idle;
        redraw = true;
      }

    float vbus = M5.Axp.GetVBusVoltage();
    if (vbus < 1.0)
    {
      if (!falte)
      {
        falte = true;
        // redraw = false;
        M5.Lcd.fillRect(10, 50, 310, 140, RED);
        M5.Lcd.textbgcolor = RED;
        M5.Lcd.textdatum = MC_DATUM;
        M5.Lcd.setFreeFont(&FreeSans12pt7b);
        M5.Lcd.drawString("Court-circuit", 320 / 2, 240 / 2 - 10);
        M5.Lcd.drawString("sur l'alimentation", 320 / 2, 240 / 2 + 10);
      }
    }
    else
    {
      if (falte)
      {
        falte = false;
        M5.Lcd.fillRect(10, 30, 310, 210, BLACK);
        redraw = true;
      }
    }

    if (redraw && !falte)
      switch (mode)
      {
      case RunMode::Idle:
        imgButOff.pushSprite((320 - ButtonSize) / 2, 30);
        break;
      case RunMode::Run:
        imgButOn.pushSprite((320 - ButtonSize) / 2, 30);
        break;
      case RunMode::Test:
      case RunMode::Move:
        imgButTest.pushSprite((320 - ButtonSize) / 2, 30);
      default:
        break;
      }

    // Redraw Banner

    if (t1 - t0 > 200UL)
    {
      float vb = M5.Axp.GetBatVoltage();
      float vbus = M5.Axp.GetVBusVoltage();

      float ib = M5.Axp.GetBatCurrent();

      sprintf(str, "Batt: %3d%%  %4.2fV  %6.1fmA  ", (int)((vb - 3.2) * 100), vb, ib);

      banner.setFreeFont(&FreeSans12pt7b);
      banner.fillSprite(BLACK);
      banner.drawString(str, 0, 0);

      banner.pushSprite(0, 0);
      t0 = t1;
    }

    M5.Lcd.fillCircle(265, 230, 10, RightInput || motorRightOn ? WHITE : DARKGREY);
    M5.Lcd.fillCircle(55, 230, 10, LeftInput || motorLeftOn ? WHITE : DARKGREY);

    M5.Lcd.drawCircle(160,230,9,RED);
    M5.Lcd.drawLine(160,230-5,160,230+5,RED);

    delay(10);
  }
}

void loopEye(void *param)
{
  const int Threshold = 3000;

  while (true)
  {
    LeftInput = analogRead(PinLeftEye) > Threshold;
    RightInput = analogRead(PinRightEye) > Threshold;

    if (mode == RunMode::Idle)
      motorLeftOn = motorRightOn = false;

    if (mode == RunMode::Move)
      motorLeftOn = motorRightOn = true;
    
    if (mode == RunMode::Run)
    {
      motorLeftOn = LeftInput;
      motorRightOn = RightInput;
    }

    delay(10);
  }
}

void setup()
{
  Serial.begin(115200);

  M5.begin(true, false, false, false);

  //M5.Axp.SetCHGCurrent(100); #teste for the usb C but not working

  for (int i = 0; i < 4; i++)
  {
    pinMode(PinMotor1[i], OUTPUT);
    pinMode(PinMotor2[i], OUTPUT);
  }

  // pinMode(PinRightEye, INPUT_PULLDOWN);
  // pinMode(PinLeftEye, INPUT_PULLDOWN);

  M5.Axp.SetCHGCurrent(4);

  xTaskCreatePinnedToCore(loopGUI, "Task GUI", 4000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopEye, "Task Eye", 4000, NULL, 0, NULL, 0);
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

    if (motorRightOn)
    {
      digitalWrite(p1[c1], HIGH);
      digitalWrite(p1[(c1 + 1) % 4], HIGH);
    }
    if (motorLeftOn)
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
