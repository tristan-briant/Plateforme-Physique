#include <M5Unified.h>
#include <button.h>
#include <limits.h>

const int ButtonSize = 180;
#define Whell_Radius 49
const float step_By_mm = 32 * 63.68395 / (Whell_Radius * PI);

void loopComunication(void *param);

Button buttonOn((320 - ButtonSize) / 2, 30, ButtonSize, ButtonSize);

M5Canvas imgButOn(&M5.Lcd);
M5Canvas imgButOff(&M5.Lcd);
M5Canvas imgButTest(&M5.Lcd);
M5Canvas banner(&M5.Lcd);

/////// Pin Config //////////
gpio_num_t PinMotor1[] = {GPIO_NUM_5, GPIO_NUM_9, GPIO_NUM_18, GPIO_NUM_17};
gpio_num_t PinMotor2[] = {GPIO_NUM_2, GPIO_NUM_1, GPIO_NUM_6, GPIO_NUM_7};

int PinRightEye = 36;
int PinLeftEye = 35;

const int N = 1000;
int TEMP = 1800;
int TEMPMIN = 1500;
int tempo = 0;

bool motorLeftOn;
bool motorRightOn;
bool send_message_flag = false;

bool LeftInput, RightInput;
bool falte;

int xleft_target, xleft;
int xright_target, xright;
float v_right, v_left;

enum RunMode
{
  Idle,
  Run,
  Test,
  Move,
  Remote
};

enum MoteurMode
{
  Speed,
  Position
};

RunMode mode = RunMode::Idle;
MoteurMode Mmode = MoteurMode::Speed;

void set_target_mm(float xr, float xl)
{
  Mmode = MoteurMode::Speed;
  mode = RunMode::Remote;

  xleft = xright = 0;

  xleft_target = (int)(xr * step_By_mm);
  xright_target = (int)(xl * step_By_mm);
}

void set_speed(float speedr, float speedl)
{
  Mmode = MoteurMode::Speed;
  mode = RunMode::Remote;

  v_right = speedr;
  v_left = speedl;
}

bool isMoving()
{
  return xleft != xleft_target || xright != xright_target;
}

bool getRightSensor()
{
  return RightInput;
}

bool getLeftSensor()
{
  return LeftInput;
}

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
      Mmode = MoteurMode::Position;
      xleft = xright = 0;
      xleft_target = xright_target = INT_MAX;
      redraw = true;
    }

    if (buttonOn.wasPressed())
    {
      if (mode != RunMode::Idle)
      {
        mode = RunMode::Idle;
        xleft = xright = xleft_target = xright_target = 0;
        // v_right = v_left = 0;
      }
      else
      {
        mode = RunMode::Run;
        Mmode = MoteurMode::Speed;
        // v1 = v2 = 100;
      }
      redraw = true;
    }

    if (M5.BtnB.isPressed())
      M5.Power.powerOff();

    if (M5.BtnA.isPressed())
    {
      mode = RunMode::Test;
      Mmode = MoteurMode::Speed;

      /*xleft = xright = 0;
      xleft_target = INT_MAX;
      xright_target = 0;*/
      v_right = 0;
      v_left = 1;
      redraw = true;
    }

    if (M5.BtnC.isPressed())
    {
      mode = RunMode::Test;
      Mmode = MoteurMode::Speed;
      xleft = xright = 0;
      xleft_target = 0;
      xright_target = INT_MAX;
      v_right = 1;
      v_left = 0;
      //  mode = RunMode::Test;
      //  motorLeftOn = false;
      //  motorRightOn = true;
      redraw = true;
    }

    if (M5.BtnA.wasReleased() || M5.BtnC.wasReleased())
    {
      mode = RunMode::Idle;
      xleft = xright = xleft_target = xright_target = 0;
      v_left = v_right = 0;
      redraw = true;
    }

    float vbus = M5.Power.getVBUSVoltage();
    if (vbus < 1.0)
    {
      if (!falte)
      {
        falte = true;
        // redraw = false;
        M5.Lcd.fillRect(10, 50, 310, 140, RED);
        // M5.Lcd.textbgcolor = RED;
        // M5.Lcd.textdatum = MC_DATUM;
        M5.Lcd.setTextColor(WHITE, RED);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setFont(&FreeSans12pt7b);
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
        imgButOn.pushSprite((320 - ButtonSize) / 2, 30);
        break;
      case RunMode::Run:
        imgButOff.pushSprite((320 - ButtonSize) / 2, 30);
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
      float vb = M5.Power.getBatteryVoltage();
      float vbus = M5.Power.getVBUSVoltage();

      float ib = M5.Power.getBatteryCurrent();

      sprintf(str, "Batt: %3d%%  %4.2fV  %6.1fmA  ", (int)((vb - 3200) * 0.1), vb / 1000, ib);

      banner.setFont(&FreeSans12pt7b);
      banner.fillSprite(BLACK);
      banner.drawString(str, 0, 0);

      banner.pushSprite(0, 0);
      t0 = t1;
    }

    M5.Lcd.fillCircle(265, 230, 10, RightInput || motorRightOn ? WHITE : DARKGREY);
    M5.Lcd.fillCircle(55, 230, 10, LeftInput || motorLeftOn ? WHITE : DARKGREY);

    M5.Lcd.drawCircle(160, 230, 9, RED);
    M5.Lcd.drawLine(160, 230 - 5, 160, 230 + 5, RED);

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

    if (mode == RunMode::Run)
    {
      Mmode = MoteurMode::Speed;
      v_right = RightInput ? 1 : 0;
      v_left = LeftInput ? 1 : 0;
    }

    delay(10);
  }
}

void setup()
{
  Serial.begin(115200);

  auto cfg = M5.config();
  cfg.output_power = true;
  M5.begin(cfg);
  Serial.begin(115200);

  // M5.Axp.SetCHGCurrent(100); #teste for the usb C but not working

  for (int i = 0; i < 4; i++)
  {
    pinMode(PinMotor1[i], OUTPUT);
    pinMode(PinMotor2[i], OUTPUT);
  }

  // pinMode(PinRightEye, INPUT_PULLDOWN);
  // pinMode(PinLeftEye, INPUT_PULLDOWN);

  // M5.Axp.SetCHGCurrent(4);

  xTaskCreatePinnedToCore(loopGUI, "Task GUI", 4000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopEye, "Task Eye", 4000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "Task Com", 4000, NULL, 0, NULL, 0);
}

void loop()
{

  unsigned int c1, c2 = 0;
  gpio_num_t *p1 = PinMotor1;
  gpio_num_t *p2 = PinMotor2;

  int directionRight = 1;
  int directionLeft = -1;

  bool target_achieved = false;

  while (true)
  { // never return
    int cc1, cc2;
    unsigned int c;
    static long t_old_r = 0, t_old_l = 0;
    long t = micros();

    /******************* MODE POSITION   **************************/
    if (Mmode == MoteurMode::Position)
    {
      target_achieved = false;
      cc1 = (c1 + directionRight) % 4;
      gpio_set_level(p1[c1], HIGH);
      gpio_set_level(p1[cc1], HIGH);
      c1 = cc1;
      xright = xright + directionRight;

      directionLeft = xleft < xleft_target ? 1 : -1;
      directionRight = xright < xright_target ? 1 : -1;

      if (xright != xright_target)
      {
        target_achieved = false;
        c = c1;
        gpio_set_level(p1[c], HIGH);
        c = (c + directionRight) % 4;
        gpio_set_level(p1[c], HIGH);
        c = (c + directionRight) % 4;
        gpio_set_level(p1[c], LOW);
        c = (c + directionRight) % 4;
        gpio_set_level(p1[c], LOW);
        c1 = (c1 + directionRight) % 4;
        xright = xright + directionRight;
      }

      if (xleft != xleft_target)
      {
        target_achieved = false;
        c = c2;
        gpio_set_level(p2[c], HIGH);
        c = (c - directionLeft) % 4; /// Signe moins : Moteur inversé
        gpio_set_level(p2[c], HIGH);
        c = (c - directionLeft) % 4;
        gpio_set_level(p2[c], LOW);
        c = (c - directionLeft) % 4;
        gpio_set_level(p2[c], LOW);
        c2 = (c2 - directionLeft) % 4; /// Signe moins : Moteur inversé
        xleft = xleft + directionLeft;
      }

      delayMicroseconds(TEMP);
    }
    else
      tempo = TEMP;

    /******************* MODE SPEED   **************************/
    if (Mmode == MoteurMode::Speed)
    {
      target_achieved = false;
      cc2 = (c2 - directionLeft) % 4; /// Signe moins : Moteur inversé
      gpio_set_level(p2[c2], HIGH);
      gpio_set_level(p2[cc2], HIGH);
      c2 = cc2;
      xleft = xleft + directionLeft;
    }
    else
      tempo = TEMP;

    tempo--;
    tempo = constrain(tempo, TEMPMIN, TEMP);

    delayMicroseconds(tempo);

    for (int i = 0; i < 4; i++)
    {
      gpio_set_level(p1[i], LOW);
      gpio_set_level(p2[i], LOW);
      directionLeft = v_left >= 0 ? 1 : -1;
      directionRight = v_right >= 0 ? 1 : -1;

      if (fabs(v_right) >= 0.01f)
      {
        if (t - t_old_r > TEMP / fabs(v_right))
        {
          c = c1;
          gpio_set_level(p1[c], HIGH);
          c = (c + directionRight) % 4;
          gpio_set_level(p1[c], HIGH);
          c = (c + directionRight) % 4;
          gpio_set_level(p1[c], LOW);
          c = (c + directionRight) % 4;
          gpio_set_level(p1[c], LOW);

          c1 = (c1 + directionRight) % 4;
          t_old_r = t;
        }
      }

      if (fabs(v_left) >= 0.01f)
      {
        if (t - t_old_l > TEMP / fabs(v_left))
        {
          c = c2;
          gpio_set_level(p2[c], HIGH);
          c = (c - directionLeft) % 4; /// Signe moins : Moteur inversé
          gpio_set_level(p2[c], HIGH);
          c = (c - directionLeft) % 4;
          gpio_set_level(p2[c], LOW);
          c = (c - directionLeft) % 4;
          gpio_set_level(p2[c], LOW);
          c2 = (c2 - directionLeft) % 4; /// Signe moins : Moteur inversé
          t_old_l = t;
        }
      }
    }

    /******************* MODE IDLE   **************************/
    if (mode == RunMode::Idle)
    {

      v_left = v_right = 0;
      xleft = xright = xleft_target = xright_target = 0;

      for (int i = 0; i < 4; i++)
      {
        gpio_set_level(p1[i], LOW);
        gpio_set_level(p2[i], LOW);
      }


      delayMicroseconds(TEMP);
    }
  }
}
