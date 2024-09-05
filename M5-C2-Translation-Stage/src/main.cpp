#include <M5Unified.h>
#include <button.h> // Home made mini lib to emulate button from M5core2 not present in M5Unified

const gpio_num_t PinDir = GPIO_NUM_19;
const gpio_num_t PinStep = GPIO_NUM_33;
const gpio_num_t PinEnable = GPIO_NUM_14;
const gpio_num_t SYNC_Pin = GPIO_NUM_26;
const gpio_num_t SYNC_REF_Pin = GPIO_NUM_25;

const bool reverse = true;

double speed = 0; // (mm/s)
const double MAXSPEED = 200;
const double ACC = 400;
const float SPEED2TIME = 200 * 32;
long synchro = 0;

int MicroStep = 64;
const float Radius = 10;
const int STEP_BY_TURN = 100;
const float MICROSTEP_BY_MM = MicroStep * STEP_BY_TURN / (2 * PI * Radius);

double x_target = 0; // mm
double x_position = 0;

enum ModeRun
{
  MOTOR_MOVING,
  MOTOR_BREAK,
  MOTOR_RELEASED,
};
ModeRun mode_run;

void TaskGUI(void *param);
void loopComunication(void *param);

void setup()
{
  auto cfg = M5.config();
  cfg.output_power = false;
  M5.begin(cfg);
  Serial.begin(115200);

  pinMode(PinDir, OUTPUT);
  pinMode(PinStep, OUTPUT);
  pinMode(PinEnable, OUTPUT);
  pinMode(SYNC_Pin, OUTPUT);
  pinMode(SYNC_REF_Pin, OUTPUT);

  xTaskCreatePinnedToCore(TaskGUI, "Task1", 20000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "", 4000, NULL, 1, NULL, 0);
}

void loop()
{
  static int s = 0;
  double x_mustep = 0;

  long t = micros();
  static long t_old = t;
  // static long t_dac = t;

  static float phase = 0;

  /*if (x_position < x_target)
    x_position += (t - t_old) * speed * 1e-6;
  if (x_position > x_target + 1e-2)
    x_position -= (t - t_old) * speed * 1e-6;
*/

  // breaking distance = 0.5 * speed^2 / ACC

  if (fabs(x_position - x_target) >= 1e-2 || fabs(speed) > 1.0)
  {
    double deltaSpeed = (t - t_old) * ACC * 1e-6;

    double x_breaking = 0.5 * speed * speed / ACC;

    if (x_target - x_position > x_breaking)  // on peut accéléré
      speed += deltaSpeed;
    else if (x_target - x_position < -x_breaking) // on peut accéléré
      speed -= deltaSpeed;
    else if (x_target - x_position > 0) // on sait qu'il faut freiner
      speed -= speed > 0 ? deltaSpeed : -deltaSpeed;
    else
      speed += speed < 0 ? deltaSpeed : -deltaSpeed;

    /*if (fabs(x_position - x_target) < 0.5 * speed * speed / ACC)
      speed -= x_position < x_target ? deltaSpeed : -deltaSpeed;
    else
      speed += x_position < x_target ? deltaSpeed : -deltaSpeed;
      */

    speed = constrain(speed, -MAXSPEED, MAXSPEED);

    x_position += (t - t_old) * speed * 1e-6;
  }

  /*if (x_position < x_target)
    x_position += (t - t_old) * speed * 1e-6;
  if (x_position > x_target + 1e-2)
    x_position -= (t - t_old) * speed * 1e-6;*/

  x_mustep = MICROSTEP_BY_MM * x_position;

  if (reverse)
    x_mustep = -x_mustep;

  t_old = t;

  if (s < x_mustep)
  {
    gpio_set_level(PinEnable, LOW);

    gpio_set_level(PinDir, HIGH);
    gpio_set_level(PinStep, HIGH);
    delayMicroseconds(1);
    gpio_set_level(PinStep, LOW);
    s++;
  }
  else if (s > x_mustep + 1.1)
  {
    gpio_set_level(PinEnable, LOW);

    gpio_set_level(PinDir, LOW);
    gpio_set_level(PinStep, HIGH);
    delayMicroseconds(1);
    gpio_set_level(PinStep, LOW);
    s--;
  }
  else
  {
    if (fabs(x_position - x_target) < 1e-2)
      gpio_set_level(PinEnable, HIGH);
  }
  /*if (t - t_dac > 500)
  {
    float y = s / MICROSTEP_BY_MM;
    dacWrite(SYNC_Pin, 128 + 127 * y);
    t_dac = t;
  }*/
}
