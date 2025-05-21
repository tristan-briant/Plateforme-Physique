#include <M5Unified.h>
#include <button.h> // Home made mini lib to emulate button from M5core2 not present in M5Unified
#include <phyphoxBle.h>

const gpio_num_t PinDir = GPIO_NUM_19;
const gpio_num_t PinStep = GPIO_NUM_33;
const gpio_num_t PinEnable = GPIO_NUM_14;
const gpio_num_t SYNC_Pin = GPIO_NUM_26;
const gpio_num_t SYNC_REF_Pin = GPIO_NUM_25;

const float SPEED_LEDC = 15;    //speed to change to ledc mode
bool modeLEDC = false;

double angle = 0, speed = 0, acc = 0.1;
double targetSpeed = 0.1, Acceleration = 0.2; // turn per second
double period = 0;

const float MAXSPEED = 10;
const double SPEED2TIME = 200 * 32;
long synchro = 0;

void TaskGUI(void *param);

enum ModeRun
{
  MOTOR_ON,
  SINUS,
  MOTOR_BREAK,
  MOTOR_RELEASED,
  PULSE // not used
};
ModeRun mode_run;

int mode = 0;

extern int delaylength;

TaskHandle_t Task1, Task2, Task3;

// float freq = 1, amp = 1, offset = 0, power = 1;
float offsetTarget = 0, offsetCal = 0;

bool isOn = true;
float SPEED_MAX = 100, ACC_MAX = 30;

int MicroStep = 64;
const float Radius = 10;
const int STEP_BY_TURN = 100;
const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);

long step;

void IRAM_ATTR countTask()
{
  if (gpio_get_level(PinDir))
    step++;
  else
    step--;
}

void setup()
{
  auto cfg = M5.config();
  cfg.output_power = false;
  M5.begin(cfg);
  Serial.begin(115200);

  // ledcSetup(0, SPEED2TIME, 8);

  // ledcChangeFrequency(0, SPEED2TIME, 8);

  // ledcWrite(0, 1);
  // ledcAttachPin(PinStep, 0);
  // modeLEDC = true;

  pinMode(PinDir, OUTPUT);
  pinMode(PinStep, OUTPUT);
  pinMode(PinEnable, OUTPUT);
  pinMode(SYNC_Pin, OUTPUT);
  pinMode(SYNC_REF_Pin, OUTPUT);

  mode_run = ModeRun::MOTOR_RELEASED;

  pinMode(26, OUTPUT);
  dacWrite(26, 200);

  dacWrite(SYNC_REF_Pin, 128);

  Serial.begin(115200);

  M5.Lcd.setBrightness(255);

  xTaskCreatePinnedToCore(TaskGUI, "Task1", 20000, NULL, 1, &Task1, 0);
  attachInterrupt(33, countTask, RISING);

  // xTaskCreatePinnedToCore(taskSetupCore1, "Task2", 2000, NULL, 1, NULL, 1);
}

long delta;

void loop()
{
  static int s = 0;
  float x = 0;

  long t = micros();
  static long t_old = t;
  static long t_dac = t;
  static double t_edge = t;

  delta = t - t_old;

  // static float phase = 0;

  if (delta > 10000)
  {

    if (mode_run == MOTOR_BREAK && fabs(speed) <= 0.0001)
    { // le plateau est à l'arrêt on relâche le moteur
      mode_run = MOTOR_RELEASED;
    }

    if (mode_run == MOTOR_RELEASED)
    {
      gpio_set_level(PinEnable, HIGH);
      t_old = t;
      speed = 0;
      step = 0;
      // return;
    }
    else
      gpio_set_level(PinEnable, LOW);

    if (mode_run == MOTOR_ON)
    {
      if (speed < targetSpeed)
      {
        speed += Acceleration * (t - t_old) * 1e-6;
        if (speed > targetSpeed)
          speed = targetSpeed;
        // period = 1e6 / fabs(speed) / SPEED2TIME;
      }
      else if (speed > targetSpeed)
      {
        speed -= Acceleration * (t - t_old) * 1e-6;
        if (speed < targetSpeed)
          speed = targetSpeed;
        // period = 1e6 / fabs(speed) / SPEED2TIME;
      }
    }

    if (mode_run == MOTOR_BREAK)
    {
      if (speed > 0)
      {
        speed -= Acceleration * (t - t_old) * 1e-6;
        if (speed < 0)
          speed = 0;
        // period = 1e6 / fabs(speed) / SPEED2TIME;
      }

      if (speed < 0)
      {
        speed += Acceleration * (t - t_old) * 1e-6;
        if (speed > 0)
          speed = 0;
        // period = 1e6 / fabs(speed) / SPEED2TIME;
      }
    }

    gpio_set_level(PinDir, speed < 0); // 0 -> CCW 1->CW

    t_old = t;
  }

  if (fabs(speed) > SPEED_LEDC) // ok never happen...
  {
    uint32_t freq = fabs(speed) * SPEED2TIME;

    if (!modeLEDC)
    {
      ledcChangeFrequency(0, freq, 8);
      ledcWrite(0, 1);
      ledcAttachPin(PinStep, 0);
      attachInterrupt(33, countTask, RISING);
      modeLEDC = true;
      t_edge = t;
    }

    if (t - t_edge > 10000)
    {
      ledcChangeFrequency(0, freq, 8);
      ledcWrite(0, 1);
      t_edge = t;
    }
  }
  else
  {
    if (modeLEDC)
    {
      ledcDetachPin(PinStep);
      gpio_set_direction(PinStep, GPIO_MODE_OUTPUT);
      attachInterrupt(33, countTask, RISING);
      modeLEDC = false;
      t_edge = t;
    }

    if (fabs(speed) > 0)
    {
      period = 1e6 / fabs(speed) / SPEED2TIME;
      while (t - t_edge > period)
      {
        gpio_set_level(PinStep, 1);
        delayMicroseconds(1);
        gpio_set_level(PinStep, 0);
        delayMicroseconds(1);
        t_edge += period;
      }
    }
    else
    {
      t_edge = t;
    }

    // ledcWrite(0, 0);
  }

  if (t - t_dac > 100)
  {
    float y = 2 * PI * ((step / 32) % 200) / 200.0;
    dacWrite(SYNC_Pin, 128 + 127 * sin(y));
    t_dac = t;
  }
}
