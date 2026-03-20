#include <M5Unified.h>
#include "PID_v1.h"
// #include "button.h"

gpio_num_t PinAH = GPIO_NUM_13,
           PinAL = GPIO_NUM_14,
           PinBH = GPIO_NUM_27,
           PinBL = GPIO_NUM_19;

int freqPWM = 10000;

bool outputEnable = false;

float I, Q;

long t_loop_us;

float turn = 0;
float turn_frac, turnTotal, turn_frac0 = 0;
float turnTotal_avg = 0;
const float epsilon = 0.02;

void loopGUI(void *param);
void loopComunication(void *param);
bool load_value(const char *name, const char *key, double *target);
void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1);  //, PID pid2);
void load_tuning(PID *pid1); //, PID *pid2);
void loopServo(void *param);
void loopSignal(void *param);

// QueueHandle_t xQueue;

double xact, xset = 20;
double output, outputGUI, res;
PID pid1(&xact, &output, &xset, 0, 0, 0, DIRECT);

void setup()
{
  auto cfg = M5.config();
  cfg.output_power = false;
  M5.begin(cfg);

  Serial.begin(115200);
  Serial2.begin(2000000, SERIAL_8N1, 33, 32);

  pinMode(PinAH, OUTPUT);
  pinMode(PinAL, OUTPUT);
  pinMode(PinBH, OUTPUT);
  pinMode(PinBL, OUTPUT);

  gpio_set_level(PinAL, 1);
  gpio_set_level(PinBL, 1);
  gpio_set_level(PinAH, 0);
  gpio_set_level(PinBH, 0);

  ledcSetup(0, freqPWM, 10);
  ledcSetup(1, freqPWM, 10);

  ledcAttachPin(PinAH, 0);
  ledcAttachPin(PinBH, 1);

  ledcWrite(0, 0);
  gpio_set_level(PinAL, 1);
  ledcWrite(1, 500);
  gpio_set_level(PinBL, 1);

  xTaskCreatePinnedToCore(loopGUI, NULL, 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopServo, "", 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "", 10000, NULL, 0, NULL, 0);

  load_tuning(&pid1);
  load_value("PID1", "XSET", &xset);
  double max = 100, min = -100;
  load_value("PID1", "OUTMAX", &max);
  load_value("PID1", "OUTMIN", &min);

  Serial.println(pid1.GetKp());
  Serial.println(pid1.GetKi());
  Serial.println(pid1.GetKd());
  Serial.println(pid1.GetDirection());
  Serial.println(pid1.GetMode());

  pid1.SetMode(MANUAL);
}

void loop()
{
  static long t_request;
  static bool data_requested = false;
  static int count = 0;
  static float I_old, Q_old;
  char x[5];

  while (true)
  {
    if (!data_requested || (micros() > t_request + 500))
    {
      data_requested = true;
      t_request = micros();
      Serial2.write(0);
    }

    // Serial2.write(0);

    if (Serial2.available())
    {
      static long t_loop_old;
      t_loop_us = micros() - t_loop_old;
      t_loop_old = micros();

      count++;
      Serial2.readBytes(x, 4);
      I = (x[0] * 256 + x[1] - 32768) / 32767.0;
      Q = (x[2] * 256 + x[3] - 32768) / 32767.0;

      if (Q >= 0 && Q_old < 0 && I < 0)
        turn--;
      if (Q < 0 && Q_old >= 0 && I < 0)
        turn++;

      turn_frac = atan2f(Q, I) / PI / 2.0;
      turnTotal = (turn + turn_frac);

      turnTotal_avg = epsilon * turnTotal + (1 - epsilon) * turnTotal_avg;

      data_requested = false;
      Q_old = Q;
      I_old = I;

      // outputGUI = output;
      delayMicroseconds(10);
    }
  }
}

void loopServo(void *param)
{

  while (true)
  {

    if (pid1.Compute())
    {
      res = pid1.getOutput();
      res = 25;

      if (outputEnable)
      {
        if (res >= 0)
        {
          gpio_set_level(PinAL, 1);
          ledcWrite(1, 0);
          delayMicroseconds(100);
          gpio_set_level(PinBL, 0);
          ledcWrite(0, fabs(res / 100.0) * 1023);
        }
        else
        {
          gpio_set_level(PinBL, 1);
          ledcWrite(0, 0);
          delayMicroseconds(100);
          gpio_set_level(PinAL, 0);
          ledcWrite(1, fabs(res / 100.0) * 1023);
        }
      }
      else
      {
        ledcWrite(0, 0);
        gpio_set_level(PinAL, 1);
        ledcWrite(1, 0);
        gpio_set_level(PinBL, 1);
      }
    }
    /*

    if (outputEnable)
    {
      gpio_set_level(PinAL, 1);
      ledcWrite(1, 0);
      delayMicroseconds(100);
      gpio_set_level(PinBL, 0);
      ledcWrite(0, 512);
    }
    else
    {
      ledcWrite(0, 0);
      gpio_set_level(PinAL, 1);
      ledcWrite(1, 0);
      gpio_set_level(PinBL, 1);
    }
*/
    outputGUI = res;
    delay(20);
  }
}
