#include <M5core2.h>

const gpio_num_t PinDir = GPIO_NUM_19;
const gpio_num_t PinStep = GPIO_NUM_33;
const gpio_num_t PinEnable = GPIO_NUM_14;
const gpio_num_t SYNC_Pin = GPIO_NUM_26;
const gpio_num_t SYNC_REF_Pin = GPIO_NUM_25;

float speed = 0;
const float MAXSPEED = 10;
const float SPEED2TIME = 200 * 32;
long synchro = 0;

enum ModeRun
{
  SINUS,
  MOTOR_BREAK,
  MOTOR_RELEASED,
  PULSE // not used
};
ModeRun mode_run;

int mode = 0;

extern int delaylength;

TaskHandle_t Task1, Task2, Task3;

float freq = 1, amp = 1, offset = 0, power = 1;
float offsetTarget = 0, offsetCal = 0;

bool isOn = true;
const float FREQ_MAX = 100, AMPL_MAX = 2000;

int MicroStep = 64;
const float Radius = 10;
const int STEP_BY_TURN = 100;
const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);

int selection = 0;
// bool selected = false;
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
  if (mode_run != PULSE)
    img.drawString("Freq ", xpos, ypos[0]);
  else
    img.drawString("Pulse press + or - ", xpos, ypos[0]);
  img.drawString("Ampl ", xpos, ypos[1]);
  // img.setTextDatum(BC_DATUM);
  img.drawString("Offset ", xpos, ypos[2]);

  img.setFreeFont(&FreeSans12pt7b);
  img.setTextDatum(CR_DATUM);
  img.setTextColor(mode_run == SINUS ? colorText : DARKGREY);
  img.setFreeFont(mode_run == SINUS ? &FreeSansBold12pt7b : &FreeSans12pt7b);
  img.drawString("On", 160 - 80, 0.5 * (ypos[2] + ypos[3]));
  img.setTextDatum(CC_DATUM);
  img.setTextColor(mode_run == MOTOR_BREAK ? colorText : DARKGREY);
  img.setFreeFont(mode_run == MOTOR_BREAK ? &FreeSansBold12pt7b : &FreeSans12pt7b);

  img.drawString("Off", 160, 0.5 * (ypos[2] + ypos[3]));
  img.setTextDatum(CL_DATUM);
  img.setTextColor(mode_run == MOTOR_RELEASED ? colorText : DARKGREY);
  img.setFreeFont(mode_run == MOTOR_RELEASED ? &FreeSansBold12pt7b : &FreeSans12pt7b);

  img.drawString("Free", 160 + 80, 0.5 * (ypos[2] + ypos[3]));

  img.setFreeFont(&FreeSans18pt7b);
  img.setTextDatum(BL_DATUM);
  img.setTextColor(colorText);
  xpos = 250;
  if (mode_run != PULSE)
    img.drawString("Hz", xpos, ypos[0]);
  img.drawString("mm", xpos, ypos[1]);
  img.drawString("mm", xpos, ypos[2]);

  img.setFreeFont(&FreeSans12pt7b);
  img.setTextDatum(BC_DATUM);
  img.drawString("select", 70, 238);
  // img.drawRect(70 - 45, 201, 90, 36, colorSelection);
  img.setFreeFont(&FreeSansBold18pt7b);
  img.drawString("-", 160, 238);
  img.drawString("+", 250, 238);

  xpos = 240;
  img.setTextDatum(BR_DATUM);
  img.setFreeFont(&FreeSansBold24pt7b);
  img.setTextColor(colorText);
  if (mode_run != PULSE)
    img.drawFloat(freq, 2, xpos, ypos[0] + 2);
  img.drawFloat(amp, 1, xpos, ypos[1] + 2);
  img.drawFloat(offsetTarget - offsetCal, 1, xpos, ypos[2] + 2);

  img.pushSprite(0, 0);
}

float xpulse = 0;
void TaskPulse(void *pvParameters)
{
  xpulse = 1;
  delay(500);
  xpulse = 0;
  vTaskDelete(NULL);
}

void TaskOffset(void *pvParameters)
{
  // Simple task for a smooth offset
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

  Button buttonFreqSelec(0, 0, 320, 50);
  Button buttonAmpSelec(0, 50, 320, 50);
  Button buttonOffsetSelec(0, 100, 320, 50);

  Button buttonOn(0, 150, 107, 50);
  Button buttonOff(107, 150, 107, 50);
  Button buttonFree(214, 150, 106, 50);

  Button buttonSelect(0, 200, 107, 50);
  Button buttonMinus(107, 200, 107, 50);
  Button buttonPlus(214, 200, 106, 50);

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
      // if (M5.BtnB.wasPressed() || (M5.BtnB.pressedFor(300) && !M5.BtnB.pressedFor(2000)) || buttonMinus.wasPressed() || (buttonMinus.pressedFor(300) && !buttonMinus.pressedFor(2000)))
      if (M5.BtnB.wasPressed() || (M5.BtnB.pressedFor(300)) || buttonMinus.wasPressed() || (buttonMinus.pressedFor(300)))
        inc = -1;
      if (M5.BtnB.pressedFor(1000) || buttonMinus.pressedFor(1000))
        inc = -10;
      if (M5.BtnB.pressedFor(3000) || buttonMinus.pressedFor(3000))
        inc = -100;
      if (M5.BtnC.wasPressed() || (M5.BtnC.pressedFor(300) ) || buttonPlus.wasPressed() || (buttonPlus.pressedFor(300) ))
        inc = +1;
      if (M5.BtnC.pressedFor(1000) || buttonPlus.pressedFor(1000))
        inc = +10;
        if (M5.BtnC.pressedFor(3000) || buttonPlus.pressedFor(3000))
        inc = +100;
    }
    if (M5.BtnA.wasPressed() || buttonSelect.wasPressed())
    {
      selection = (selection + 1) % NSELECTION;
      draw();
    }

    if (buttonFreqSelec.isPressed())
    {
      selection = 0;
      draw();
    }
    if (buttonAmpSelec.isPressed())
    {
      selection = 1;
      draw();
    }
    if (buttonOffsetSelec.isPressed())
    {
      selection = 2;
      draw();
    }
    if (buttonOn.isPressed())
    {
      mode_run = ModeRun::SINUS;
      draw();
    }
    if (buttonOff.isPressed())
    {
      mode_run = ModeRun::MOTOR_BREAK;
      draw();
    }
    if (buttonFree.isPressed())
    {
      mode_run = ModeRun::MOTOR_RELEASED;
      draw();
    }

    if (inc != 0)
    {
      switch (selection)
      {
      case 0:
        if (mode_run != PULSE)
          freq = constrain(freq + inc * 0.01, 0, FREQ_MAX);
        else if (xpulse == 0)
          xTaskCreatePinnedToCore(TaskPulse, "Task3", 4000, NULL, 1, &Task3, 0);

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
    delay(10);
    M5.update();
  }
}

void setup()
{

  // M5.begin();
  M5.Axp.begin();
  // comment ligne "axp192.begin();" in Axp.cpp (ligne 113)

  Wire1.begin(21, 22);
  Wire1.setClock(100000);
  M5.Axp.SetESPVoltage(3350);
  Serial.printf("axp: esp32 power voltage was set to 3.35v\n");

  M5.Axp.SetLcdVoltage(2800);
  Serial.printf("axp: lcd backlight voltage was set to 2.80v\n");

  M5.Axp.SetLDOVoltage(2, 3300); // Periph power voltage preset (LCD_logic, SD card)
  Serial.printf("axp: lcd logic and sdcard voltage preset to 3.3v\n");

  M5.Axp.SetLDOVoltage(3, 2000); // Vibrator power voltage preset
  Serial.printf("axp: vibrator voltage preset to 2v\n");

  M5.Axp.SetLDOEnable(2, true);
  M5.Axp.SetDCDC3(true); // LCD backlight
  M5.Axp.SetLed(true);
  M5.Axp.SetDCDC3(true);
  M5.Lcd.begin();

  pinMode(PinDir, OUTPUT);
  pinMode(PinStep, OUTPUT);
  pinMode(PinEnable, OUTPUT);
  pinMode(SYNC_Pin, OUTPUT);
  pinMode(SYNC_REF_Pin, OUTPUT);

  // pinMode(25, OUTPUT); // quiet speaker
  pinMode(26, OUTPUT);
  dacWrite(26, 200);

  dacWrite(SYNC_REF_Pin, 128);

  img.createSprite(320, 240); // Create a 320x240 canvas

  Serial.begin(115200);

  M5.Lcd.setBrightness(255);

  xTaskCreatePinnedToCore(TaskGUI, "Task1", 20000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(TaskOffset, "Task2", 20000, NULL, 1, &Task2, 0);

  draw();
}

void loop()
{
  static int s = 0;
  float x = 0;

  long t = micros();
  static long t_old = t;
  static long t_dac = t;

  static float phase = 0;

  if (mode_run == MOTOR_RELEASED)
  {
    gpio_set_level(PinEnable, HIGH);
    return;
  }
  else
    gpio_set_level(PinEnable, LOW);

  if (mode_run == SINUS || (mode_run == MOTOR_BREAK && abs(sin(phase)) > 0.01))
  {
    phase += 2 * PI * freq * (t - t_old) / 1000000;

    if (phase > 2 * PI)
      phase -= 2 * PI;
  }
  x = MICROSTEP_BY_MM * (amp * sin(phase) + offset);

  if (mode_run == PULSE)
  {
    x = MICROSTEP_BY_MM * (amp * (sin(phase) + xpulse) + offset);
  }

  t_old = t;

  if (s < x)
  {
    gpio_set_level(PinDir, HIGH);
    gpio_set_level(PinStep, HIGH);
    delayMicroseconds(1);
    gpio_set_level(PinStep, LOW);
    s++;
  }
  else if (s > x + 1)
  {
    gpio_set_level(PinDir, LOW);
    gpio_set_level(PinStep, HIGH);
    delayMicroseconds(1);
    gpio_set_level(PinStep, LOW);
    s--;
  }

  if (t - t_dac > 500)
  {
    float y = s / MICROSTEP_BY_MM / amp - offset;
    dacWrite(SYNC_Pin, 128 + 127 * y);
    t_dac = t;
  }
}
