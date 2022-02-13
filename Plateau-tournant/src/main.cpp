#include <Arduino.h>
#include <M5Stack.h>

void oneStep(int forward);
void oneStep2(bool forward);
void oneStep3(bool forward);
void oneStep4(bool forward);
void oneStep5(bool forward);
void motor_init();
void ReleaseMotor();

enum ModeRun
{
  MOTOR_ON,
  MOTOR_RELEASED,
};
ModeRun mode_run;

int mode = 0;

extern int delaylength;

const int SYNC_Pin = 22;

TaskHandle_t Task1, Task2, Task3;

float freq = 1, amp = 1, offset = 0, power = 1;
float offsetTarget = 0, offsetCal = 0;

bool isOn = true;
const float FREQ_MAX = 10, AMPL_MAX = 2000;

int MicroStep = 64;
const float Radius = 10;
const int STEP_BY_TURN = 100;
const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);

int selection = 0;
//bool selected = false;
const int NSELECTION = 4;

int colorSelection = DARKGREEN;
int colorText = WHITE;
int colorBackGround = BLACK;

TFT_eSprite img = TFT_eSprite(&M5.Lcd);

void draw()
{
  int ypos[] = {50, 100, 150, 200};
  int xpos;

  img.fillSprite(colorBackGround);
  img.setTextSize(1);
  img.setTextColor(colorText);

  for (int i = 0; i < NSELECTION; i++)
    img.drawRect(5, ypos[i] - 50, 320 - 10, 50, colorSelection);

  img.fillRect(5, ypos[selection] - 50, 320 - 10, 50, colorSelection);

  xpos = 10;
  img.setTextDatum(BL_DATUM);
  img.setFreeFont(&FreeSans18pt7b);
  
  img.drawString("Speed ", xpos, ypos[1]);
  //img.setTextDatum(BC_DATUM);
 

  img.setFreeFont(&FreeSans12pt7b);
  img.setTextDatum(CR_DATUM);
  img.setTextColor(mode_run == MOTOR_ON ? colorText : DARKGREY);
  img.drawString("On", 160 - 80, 0.5 * (ypos[2] + ypos[3]));
  img.setTextDatum(CL_DATUM);
  img.setTextColor(mode_run == MOTOR_RELEASED ? colorText : DARKGREY);
  img.drawString("Free", 160 + 80, 0.5 * (ypos[2] + ypos[3]));

  img.setFreeFont(&FreeSans18pt7b);
  img.setTextDatum(BL_DATUM);
  img.setTextColor(colorText);
  xpos = 250;
  
  img.drawString("rad/s", xpos, ypos[0]);
  
  img.setFreeFont(&FreeSans12pt7b);
  img.setTextDatum(BC_DATUM);
  img.drawString("select", 70, 238);
  //img.drawRect(70 - 45, 201, 90, 36, colorSelection);
  img.setFreeFont(&FreeSansBold18pt7b);
  img.drawString("-", 160, 238);
  img.drawString("+", 250, 238);

  xpos = 240;
  img.setTextDatum(BR_DATUM);
  img.setFreeFont(&FreeSansBold24pt7b);
  img.setTextColor(colorText);
 
  img.drawFloat(amp, 1, xpos, ypos[1] + 2);
  img.drawFloat(offsetTarget - offsetCal, 1, xpos, ypos[2] + 2);

  img.pushSprite(0, 0);
}

void TaskOffset(void *pvParameters)
{
  //Simple task for a smooth offset
  float epsilon = 0.02;
  while (true)
  {
    if (offset < offsetTarget)
      offset += epsilon;
    else if (offset >= offsetTarget + epsilon)
      offset -= epsilon;

    delay(1);
  }
}

void TaskGUI(void *pvParameters)
{

  while (true)
  {
    int inc = 0;

    if (M5.BtnB.isPressed() && M5.BtnC.isPressed())
    {
      offsetCal = offsetTarget;
      draw();
    }
    else
    {
      if (M5.BtnB.wasPressed() || (M5.BtnB.pressedFor(300) && !M5.BtnB.pressedFor(2000)))
        inc = -1;
      if (M5.BtnB.pressedFor(2000))
        inc = -5;
      if (M5.BtnC.wasPressed() || (M5.BtnC.pressedFor(300) && !M5.BtnC.pressedFor(2000)))
        inc = +1;
      if (M5.BtnC.pressedFor(2000))
        inc = +5;
    }
    if (M5.BtnA.wasPressed())
    {
      selection = (selection + 1) % NSELECTION;
      draw();
    }
    if (inc != 0)
    {
      switch (selection)
      {
      case 0:
       
          freq = constrain(freq + inc * 0.01, 0, FREQ_MAX);
       
        break;
      case 1:
        amp = constrain(amp + constrain(inc, -2, +2) * 0.1, 0, AMPL_MAX);
        break;
      case 2:
        offsetTarget += constrain(inc, -1, +1) * 0.5;
        break;
      case 3:
        mode_run = (ModeRun)constrain(mode_run + constrain(inc, -1, +1), 0, 2);
      }
      draw();
    }
    delay(1);
    M5.update();
  }
}

void setup()
{
  M5.begin();

  pinMode(25, OUTPUT); // for a quiet speaker

  pinMode(SYNC_Pin, OUTPUT);

  motor_init();

  img.setColorDepth(8);
  img.setTextColor(TFT_WHITE);
  img.createSprite(320, 240);

  Serial.begin(115200);

  M5.Lcd.setBrightness(255);

  xTaskCreatePinnedToCore(TaskGUI, "Task1", 20000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(TaskOffset, "Task2", 20000, NULL, 1, &Task2, 0);

  draw();
}

void loop()
{
 
    oneStep(1);
  
}
