#include <M5Unified.h>
#include <button.h> // Home made mini lib to emulate button from M5core2 not present in M5Unified
// #include <phyphoxBle.h>

const gpio_num_t PinDir = GPIO_NUM_19;
const gpio_num_t PinStep = GPIO_NUM_33;
const gpio_num_t PinEnable = GPIO_NUM_14;
const gpio_num_t SYNC_Pin = GPIO_NUM_26;
const gpio_num_t SYNC_REF_Pin = GPIO_NUM_25;

float speed = 0;
const float MAXSPEED = 10;
const float SPEED2TIME = 200 * 32;
long synchro = 0;

void TaskGUI(void *param);

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
float FREQ_MAX = 100, AMPL_MAX = 2000;

int MicroStep = 32;
const float Radius = 10;
const int STEP_BY_TURN = 400;
const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);

void TaskOffset(void *pvParameters)
{
  // Simple task for a smooth offset
  long t_old = micros();

  // float speedMax = 50 * 1e-6; // en mm/µs
  float speed_offset = 5 * 1e-6; // en mm/µs

  while (true)
  {
    long t = micros();
    float dx = speed_offset * (t - t_old);

    if (offset < offsetTarget)
      offset += dx;
    else if (offset >= offsetTarget + dx)
      offset -= dx;

    t_old = t;
    delay(10);
  }
}

void setup()
{
  auto cfg = M5.config();
  cfg.output_power = false;
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Lcd.setBrightness(255);

  pinMode(PinDir, OUTPUT);
  pinMode(PinStep, OUTPUT);
  pinMode(PinEnable, OUTPUT);
  pinMode(SYNC_Pin, OUTPUT);
  pinMode(SYNC_REF_Pin, OUTPUT);

  mode_run = ModeRun::MOTOR_RELEASED;

  pinMode(26, OUTPUT);
  dacWrite(26, 200);

  dacWrite(SYNC_REF_Pin, 128);

  xTaskCreatePinnedToCore(TaskGUI, "Task1", 20000, NULL, 1, &Task1, 0);
  //xTaskCreatePinnedToCore(TaskOffset, "Task2", 20000, NULL, 1, &Task2, 0);
}

void loop()
{
  static int s = 0;
  float x = 0;

  long t = micros();
  static long t_old = t;
  static long t_dac = t;

  static float phase = 0;
  float speed_offset = 20 * 1e-6; // en mm/µs

  float dx = speed_offset * (t - t_old);

  if (offset < offsetTarget)
    offset += dx;
  else if (offset >= offsetTarget + dx)
    offset -= dx;

  if (mode_run == MOTOR_RELEASED)
  {
    gpio_set_level(PinEnable, HIGH);
    t_old = t;
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

  if (mode_run == PULSE)
  {
    phase += 2 * PI * freq * (t - t_old) / 1000000;

    if (phase > 2 * PI)
    {
      phase -= 2 * PI;
      mode_run = MOTOR_BREAK;
    }
  }

  x = MICROSTEP_BY_MM * (amp * sin(phase) + offset);

  t_old = t;

  if (s < x)
  {
    gpio_set_level(PinDir, LOW);
    gpio_set_level(PinStep, HIGH);
    delayMicroseconds(1);
    gpio_set_level(PinStep, LOW);
    s++;
  }
  else if (s > x + 1)
  {
    gpio_set_level(PinDir, HIGH);
    gpio_set_level(PinStep, HIGH);
    delayMicroseconds(1);
    gpio_set_level(PinStep, LOW);
    s--;
  }

  delayMicroseconds(1);

  if (t - t_dac > 500)
  {
    float y = sin(phase);              // s / MICROSTEP_BY_MM / amp - offset;
    dacWrite(SYNC_Pin, 128 + 116 * y); // 116 =>1.5V d'amplitude (éviter 127 car saturation) //116 =>1.5V d'amplitude (éviter 127 car saturation)
    t_dac = t;
  }
}
