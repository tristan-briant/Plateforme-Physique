// #include "M5Dial.h"
#include "Encoder.h"
#include "driver/ledc.h"
#include "M5Unified.h"
#include "motor.h"
#include "dessin.h"
#include "button.h"

/*const gpio_num_t PinDir = GPIO_NUM_13;
const gpio_num_t PinStep = GPIO_NUM_15;
const gpio_num_t PinEnable = GPIO_NUM_1;
const gpio_num_t PinContact = GPIO_NUM_2;*/

const u_char Z_LIMIT_UP_MASK = 1;
const u_char Z_LIMIT_DOWN_MASK = 2;
const u_char PEN_MASK = 8; /// Normalement 4 mais la pin GPA2 est cramée!!

double Z_PEN_HEIGHT = 10.0; // Height of the contact sensor

float increment[] = {0.1, 1.0, 10.0};
int incrementNUM = 3;

int sensor;

bool motorRunning = false;

bool ZLimitUP = false, ZLimitDOWN = false, ContactPen = false;
bool ResetPosition = false;

// const bool reverse = true;
// const bool CCW_DIAL = true; // DIAL in CW or CCW (true if DINMETER false if M5DIAL)

double speed = 0; // (mm/s)
const double MAXSPEED = 2;
const double ACC = 2;
const double BREAK = 8;

// int MicroStep = 64;
//  const float Radius = 10;
const int STEP_BY_TURN = 1000;
// const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);
const float MM_BY_TURN = 4.0;
const float MICROSTEP_BY_MM = STEP_BY_TURN / MM_BY_TURN;

double x_target = 0; // mm
double x_position = 0;

enum ModeRun
{
  MOTOR_MOVING,
  MOTOR_BREAK,
  MOTOR_RELEASED,
  AUTOFOCUS,
  LEARNING
};
ModeRun mode_run;

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
  return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3;
}

void StopMotor()
{
  x_target = x_position;
  speed = 0;
  mode_run = MOTOR_RELEASED;
}

void loopGUI(void *param);
void save_ZPEN(double zpen);
double load_ZPEN();

void setup()
{
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  Z_PEN_HEIGHT = load_ZPEN();

  M5.Lcd.setRotation(0);

  beginMCP();

  xTaskCreatePinnedToCore(loopGUI, NULL, 4000, NULL, 0, NULL, 0);
}

void loop()
{
  static int s = 0;
  static double x_mustep = 0;

  long t = micros();
  static long t_old = t;

  sensor = readMCP();
  // sensor = 0;
  ZLimitDOWN = !(sensor & Z_LIMIT_DOWN_MASK);
  ZLimitUP = !(sensor & Z_LIMIT_UP_MASK);
  ContactPen = sensor & PEN_MASK;

  if (mode_run == ModeRun::AUTOFOCUS && ContactPen)
  {
    mode_run = ModeRun::MOTOR_MOVING;
    x_position = Z_PEN_HEIGHT;
    s = x_mustep = MICROSTEP_BY_MM * x_position;
    x_target = 0;
  }
  else if (mode_run == ModeRun::LEARNING && ContactPen)
  {
    save_ZPEN(x_position);
    mode_run = ModeRun::MOTOR_MOVING;
    Z_PEN_HEIGHT = x_position;
    // s = x_mustep = MICROSTEP_BY_MM * x_position;
    x_target = 0;
  }
  else if ((ZLimitUP || ContactPen) && x_position < x_target)
  {
    x_target = x_position;
  }
  else if (ZLimitDOWN && x_position > x_target)
  {
    x_target = x_position;
  }

  if (ResetPosition)
  {
    x_target = x_position = 0;
    s = x_mustep = 0;
    ResetPosition = false;
  }

  if (fabs(x_position - x_target) >= 1e-2 || fabs(speed) > 1.0 || fabs(s - x_mustep) > 1 || fabs(x_mustep - s) > 1)
  {
    motorRunning = true;

    double deltaSpeed = (t - t_old) * ACC * 1e-6;
    double x_breaking = 0.5 * speed * speed / BREAK; // breaking distance = 0.5 * speed^2 / BREAK

    if (x_target - x_position > x_breaking && speed > 0) // on peut accélérer
      speed += deltaSpeed;
    else if (x_target - x_position < -x_breaking && speed < 0) // on peut accélérer
      speed -= deltaSpeed;
    else
    {
      deltaSpeed = (t - t_old) * BREAK * 1e-6;
      if (x_target - x_position > 0) // on sait qu'il faut freiner
        speed -= speed > 0 ? deltaSpeed : -deltaSpeed;
      else
        speed += speed < 0 ? deltaSpeed : -deltaSpeed;
    }

    speed = constrain(speed, -MAXSPEED, MAXSPEED);

    x_position += (t - t_old) * speed * 1e-6;
    x_mustep = MICROSTEP_BY_MM * x_position;

    if (s < x_mustep)
    {
      step(1);
      s++;
    }
    else if (s > x_mustep + 1)
    {
      step(-1);
      s--;
    }
  }
  else
  {
    motorRunning = false;
    speed = 0;
    stopMotor();
  }

  t_old = t;
}

void loopGUI(void *param)
{
  Button buttonINC(0, 80, 60, 80);
  Button buttonMAIN(96, 56, 128, 128);

  M5Canvas img(&M5.Lcd);
  img.createSprite(240, 240);

  init_encoder();
  M5.Speaker.setVolume(30);

  int INC_select = 0;

  int c_enc;
  char str[20];

  int color1 = color565(255, 128, 0);    // Orange
  int color2 = color565(100, 100, 100);  // Gris
  int color4 = color565(200, 200, 200);  // Gris clair
  int color3 = color565(255, 0, 0);      // RED
  int color5 = color565(50, 50, 50);     // Gris
  int colorFocus = color565(190, 95, 0); // Orange foncé
  int colorLearn = color565(0, 150, 0);  // Vert

  long t_blink = 0;
  bool blink;

  while (true)
  {
    long t = millis();
    if (t - t_blink > 500)
    {
      t_blink = t;
      blink = !blink;
    }

    ///////////// Handle Buttons //////////////////////
    M5.update();

    if (buttonINC.wasPressed())
    {
      M5.Speaker.tone(1000, 10);

      INC_select = INC_select + 1;
      if (INC_select >= incrementNUM)
        INC_select = 0;
    }

    if (buttonMAIN.wasPressed())
    {
      StopMotor();
      M5.Speaker.tone(1000, 10);
    }

    bool longpressed;
    if (buttonMAIN.pressedFor(1000) && !longpressed)
    {
      ResetPosition = true;
      M5.Speaker.tone(3000, 100);
      longpressed = true;
    }
    if (buttonMAIN.wasReleased())
      longpressed = false;

    if (M5.BtnA.wasClicked())
    {
      if (mode_run == AUTOFOCUS)
      {
        StopMotor();
      }
      else
      {
        mode_run = AUTOFOCUS;
        x_target += 40;
      }
    }

    // set the ZPEN HEIGH
    if (M5.BtnA.pressedFor(3000) && !longpressed)
    {
      M5.Speaker.tone(3000, 100);
      StopMotor();
      ResetPosition = true;
      delay(100);
      mode_run = LEARNING;
      x_target = 20;
      // save_ZPEN(x_position);
      // Z_PEN_HEIGHT = load_ZPEN();
      longpressed = true;
    }
    if (M5.BtnA.wasReleased())
      longpressed = false;

    /////////////////// Handle Encoder /////////////////////

    int cnew = get_encoder();

    if (fabs(cnew) >= 4)
    {
      c_enc = c_enc + cnew > 0 ? +1 : -1;
      clear_encoder();
      if (ZLimitUP && c_enc > 0)
        c_enc = 0;
      if (ZLimitDOWN && c_enc < 0)
        c_enc = 0;

      x_target += c_enc * increment[INC_select];
      M5.Speaker.tone(1000, 10);
    }

    ///////////////// DRAW /////////////////////////

    bool DiplayStop = motorRunning && fabs(x_position - x_target) > 0.2;

    int colsquare = DiplayStop ? color3 : color1;

    if (mode_run == ModeRun::AUTOFOCUS)
      colsquare = colorFocus;
    if (mode_run == ModeRun::LEARNING)
      colsquare = colorLearn;

    img.fillScreen(BLACK);
    img.drawXBitmap(0, 0, arrowUP_bits, 80, 120, (ZLimitUP || ContactPen) ? color5 : color1);
    img.drawXBitmap(0, 120, arrowDOWN_bits, 80, 120, ZLimitDOWN ? color5 : color1);

    img.fillRoundRect(2, 96, 48, 48, 4, color2);
    img.fillRoundRect(96, 56, 128, 128, 20, colsquare);

    img.fillRoundRect(92, 220, 56, 48, 4, color2);
    img.setFont(&FreeSans9pt7b);
    img.setTextDatum(CC_DATUM);
    img.setTextColor(WHITE, color2);
    img.drawString("AUTO", 96 + 24, 230);

    img.setFont(&FreeSansBold24pt7b);
    img.setTextColor(WHITE, colsquare);
    img.setTextDatum(CC_DATUM);

    if (mode_run == ModeRun::AUTOFOCUS)
    {
      img.setFont(&FreeSansBold18pt7b);
      img.drawString("FOCUS", 96 + 64, 120);
    }
    else if (mode_run == ModeRun::LEARNING)
    {
      img.setFont(&FreeSansBold18pt7b);
      img.drawString("SET", 96 + 64, 120);
    }
    else
    {
      sprintf(str, "%.1f", x_target);
      img.drawString(str, 96 + 64, 120);
    }

    if (DiplayStop)
    {
      img.setFont(&FreeSans12pt7b);
      img.setTextColor(color4, colsquare);
      img.setTextDatum(CC_DATUM);
      sprintf(str, "%.1f", x_position);
      img.drawString(str, 96 + 64, 160);
      // if (blink)
      {
        img.setTextColor(WHITE, colsquare);
        img.drawString("STOP", 96 + 64, 80);
      }
    }

    img.setFont(&FreeSans12pt7b);
    img.setTextColor(WHITE, color2);
    img.setTextDatum(CC_DATUM);
    if (increment[INC_select] < 1)
      sprintf(str, "%.1f", increment[INC_select]);
    else
      sprintf(str, "%.0f", increment[INC_select]);
    img.drawString(str, 2 + 24, 120);

    img.setFont(&FreeSansBold9pt7b);
    img.setTextDatum(CC_DATUM);
    img.setTextColor(RED, BLACK);

    if (blink)
    {
      if (!(sensor & Z_LIMIT_UP_MASK))
        img.drawString("LIMIT UP", 120, 20);
      if (!(sensor & Z_LIMIT_DOWN_MASK))
        img.drawString("LIMIT DOWN", 120, 20);
      if (sensor & PEN_MASK)
        img.drawString("PEN", 120, 20);
    }

    img.pushSprite(0, 0);

    delay(10);
  }
}