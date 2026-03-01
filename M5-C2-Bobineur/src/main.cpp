#include <M5Unified.h>
// #include <button.h> // Home made mini lib to emulate button from M5core2 not present in M5Unified
//  #include <phyphoxBle.h>

const gpio_num_t PinDir = GPIO_NUM_19;
const gpio_num_t PinStep = GPIO_NUM_33;
const gpio_num_t PinEnable = GPIO_NUM_14;
const gpio_num_t SYNC_Pin = GPIO_NUM_26;
const gpio_num_t SYNC_REF_Pin = GPIO_NUM_25;

const float SPEED_LEDC = 15; // speed to change to ledc mode
bool modeLEDC = false;

double speed = 0; // Actual speed (turn per second)
double targetSpeed = 1;
double Acceleration = 4;                             // turn per second
double TargetTurn = 10, TurnToTarget, TotalTurn = 0; // number of turn

const double SPEED2TIME = 200 * 32; // Conversion ratio: microstep by turn
long synchro = 0;

void TaskGUI(void *param);

enum ModeRun
{
  MOTOR_ON,
  MODE_TURN,
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
float SPEED_MAX = 3, ACC_MAX = 30;

const int MicroStep = 64;
const int STEP_BY_TURN = 100;
const float Radius = 10;
const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);

long step, step_old, step_offset;

void IRAM_ATTR countTask()
{
  if (gpio_get_level(PinDir))
    step--;
  else
    step++;
}

void setup()
{
  auto cfg = M5.config();
  cfg.output_power = false;
  M5.begin(cfg);
  Serial.begin(115200);
  Serial2.begin(1000000, SERIAL_8N1, -1, 32, false);

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
}

void loop()
{
  static int s = 0;
  float x = 0;

  unsigned long t = micros();
  static unsigned long t_old = t;
  static unsigned long t_dac = t;
  static unsigned long t_edge = t;

  unsigned long delta = t - t_old;
  unsigned long period; // Period of pulses in micros

  if (delta > 10000UL) // Comparaison en UL pour être valide même avec les overflows de micros()
  {
    if (mode_run == MOTOR_BREAK && fabs(speed) <= 0.0001)
    { // le plateau est à l'arrêt on relâche le moteur
      mode_run = MOTOR_RELEASED;
    }

    if (mode_run == MOTOR_RELEASED)
    {
      gpio_set_level(PinEnable, HIGH);
      speed = 0;
    }

    if (mode_run == MOTOR_ON || mode_run == MODE_TURN)
    {
      gpio_set_level(PinEnable, LOW);
      gpio_set_level(PinDir, speed < 0); // 0 -> CCW 1->CW

      if (speed < targetSpeed)
        speed = min(speed + Acceleration * (t - t_old) * 1e-6, targetSpeed);

      else if (speed > targetSpeed)
        speed = max(speed - Acceleration * (t - t_old) * 1e-6, targetSpeed);
    }

    if (mode_run == MOTOR_BREAK)
    {
      gpio_set_level(PinEnable, LOW);
      gpio_set_level(PinDir, speed < 0); // 0 -> CCW 1->CW

      if (speed > 0)
        speed = max(speed - Acceleration * (t - t_old) * 1e-6, 0.0);

      if (speed < 0)
        speed = max(speed + Acceleration * (t - t_old) * 1e-6, 0.0);
    }

    if (mode_run == MODE_TURN)
    {
      gpio_set_level(PinEnable, LOW);
      gpio_set_level(PinDir, speed < 0); // 0 -> CCW 1->CW

      if (abs(TurnToTarget) < 0.5 * abs(speed / Acceleration))
        mode_run = MOTOR_BREAK;
      else
      {
        TurnToTarget = TargetTurn - abs((double)(step - step_offset)) / STEP_BY_TURN / MicroStep;
      }
    }

    t_old = t;
    step_old = step;
  }

  static double period_double = 0;
  static double cumul = 0; // cumulated error

  if (fabs(speed) > 0)
  {
    period_double = 1e6 / fabs(speed) / SPEED2TIME;
    period = (unsigned long)period_double;

    while (t - t_edge > period)
    {
      gpio_set_level(PinStep, 1);
      delayMicroseconds(1);
      gpio_set_level(PinStep, 0);
      delayMicroseconds(1);
      t_edge += period;

      cumul += period_double - period;
      unsigned long lc = (unsigned long)cumul;
      if (lc > 0)
      {
        t_edge += lc;
        cumul -= lc;
      }
    }
  }
  else
  {
    t_edge = t;
  }

  TotalTurn = (double)step / STEP_BY_TURN / MicroStep;

  if (t - t_dac > 100UL)
  {
    float y = 2 * PI * ((step / 32) % 200) / 200.0;
    dacWrite(SYNC_Pin, 128 + 127 * sin(y));
    t_dac = t;
  }
}
