#include <M5Unified.h>
#include <button.h> // Home made mini lib to emulate button from M5core2 not present in M5Unified
#include <phyphoxBle.h>

const gpio_num_t PinDir = GPIO_NUM_19;
const gpio_num_t PinStep = GPIO_NUM_33;
const gpio_num_t PinEnable = GPIO_NUM_14;
const gpio_num_t SYNC_Pin = GPIO_NUM_26;
const gpio_num_t SYNC_REF_Pin = GPIO_NUM_25;

float speed = 0;
const float MAXSPEED = 10;
const float SPEED2TIME = 200 * 32;
long synchro = 0;

void TaskGUI(void* param);

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

int MicroStep = 64;
const float Radius = 10;
const int STEP_BY_TURN = 100;
const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);



void TaskOffset(void *pvParameters)
{
  // Simple task for a smooth offset
  long t_old = micros();

  float speedMax = 50 * 1e-6; // en mm/µs
  float speed = 50 * 1e-6;    // en mm/µs

  while (true)
  {
    long t = micros();
    float dx = speed * (t - t_old);

    if (offset < offsetTarget)
      offset += dx;
    else if (offset >= offsetTarget + dx)
      offset -= dx;

    t_old = t;
    delay(1);
  }
}


void setup()
{
  auto cfg = M5.config();
  cfg.output_power = false;
  M5.begin(cfg);
  Serial.begin(115200);
  // axp192.SetBusPowerMode(kMBusModeInput);
  // M5.begin(true, true, true, true, kMBusModeInput, true);
  // M5.Axp.begin();
  // comment ligne "axp192.begin();" in Axp.cpp (ligne 113)

  // Wire1.begin(21, 22);
  // Wire1.setClock(100000);
  /*M5.Axp.SetESPVoltage(3350);
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
  M5.Lcd.begin();*/

  pinMode(PinDir, OUTPUT);
  pinMode(PinStep, OUTPUT);
  pinMode(PinEnable, OUTPUT);
  pinMode(SYNC_Pin, OUTPUT);
  pinMode(SYNC_REF_Pin, OUTPUT);

  // pinMode(25, OUTPUT); // quiet speaker
  mode_run = ModeRun::MOTOR_RELEASED;

  pinMode(26, OUTPUT);
  dacWrite(26, 200);

  dacWrite(SYNC_REF_Pin, 128);

  // img.createSprite(320, 240); // Create a 320x240 canvas

  Serial.begin(115200);

  M5.Lcd.setBrightness(255);

  xTaskCreatePinnedToCore(TaskGUI, "Task1", 20000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(TaskOffset, "Task2", 20000, NULL, 1, &Task2, 0);

  // draw();
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
