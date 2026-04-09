#include <M5Unified.h>
#include "PID_v1.h"
// #include "button.h"

// Board version
// #define BOARD_V1   //fisrt version with cms transisto
#define BOARD_V2 // newer version with TTL 7402N chip

#ifdef BOARD_V1
gpio_num_t PinAH = GPIO_NUM_13,
           PinAL = GPIO_NUM_14,
           PinBH = GPIO_NUM_27,
           PinBL = GPIO_NUM_19;
#endif

#ifdef BOARD_V2
gpio_num_t PinPWM = GPIO_NUM_19,
           PinDIR = GPIO_NUM_27;

gpio_num_t PinISENS = GPIO_NUM_35;
gpio_num_t PinInput = GPIO_NUM_36;
#endif

int freqPWM = 10000;

bool outputEnable = false;

float I, Q;
float Isens;
float VInput;

const float LambdaOver2 = 340.0 / 40 * 0.5; // lambda over 2 in mm ( 0.5 * c / freq *1000)  with f= 40kHz

bool Detecteur_AUX = false; // True if a auxiliary detector (ie ultrasonic) is plugged on port A

long t_loop_us;

float turn = 0;
float turn_frac, turnTotal, turn_frac0;
float turnTotal_avg = 0;
const float epsilon = 0.02;

void loopGUI(void *param);
void loopComunication(void *param);
void loopISens(void *param);

bool load_value(const char *name, const char *key, double *target);
void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1);  //, PID pid2);
void load_tuning(PID *pid1); //, PID *pid2);
void loopServo(void *param);
// void loopSignal(void *param);

double xact, xset;
double output, outputGUI, res;
PID pid1(&xact, &output, &xset, 0, 0, 0, DIRECT);

float mediane(float a, float b, float c)
{
  float x;

  if (a > b)
  {
    x = a;
    a = b;
    b = x;
  }
  if (c > b)
    return b;
  else if (c > a)
    return c;
  else
    return a;
}

void setup()
{
  auto cfg = M5.config();
  cfg.output_power = false;
  M5.begin(cfg);

  Serial.begin(115200);
  Serial2.begin(2000000, SERIAL_8N1, 33, 32);

#ifdef BOARD_V1
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
  ledcWrite(1, 0);
  gpio_set_level(PinBL, 1);
#endif
#ifdef BOARD_V2
  pinMode(PinDIR, OUTPUT);
  pinMode(PinPWM, OUTPUT);

  ledcSetup(0, freqPWM, 10);
  ledcAttachPin(PinPWM, 0);
  ledcWrite(0, 1023);

  adcAttachPin(PinISENS);
  adcAttachPin(PinInput);
  analogReadResolution(12);
  analogSetClockDiv(1);
  analogSetAttenuation(ADC_11db);



#endif

  xTaskCreatePinnedToCore(loopGUI, NULL, 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopServo, "", 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "", 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopISens, "", 4000, NULL, 1, NULL, 0);

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
  pid1.SetSampleTime(10);
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
      Detecteur_AUX = true;
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

      xact = -(turnTotal_avg - turn_frac0) * LambdaOver2;

      data_requested = false;
      Q_old = Q;
      I_old = I;

      output = 0;
      // outputGUI = output;
      delayMicroseconds(10);
    }
  }
}

void loopServo(void *param)
{

  while (true)
  {

#ifdef BOARD_V1
    if (outputEnable)
    {
      pid1.Compute();
      res = pid1.getOutput();

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

      else
      {
        res = 0;
        ledcWrite(0, 0);
        gpio_set_level(PinAL, 1);
        ledcWrite(1, 0);
        gpio_set_level(PinBL, 1);
      }
    }
#endif

#ifdef BOARD_V2
    if (outputEnable)
    {
      pid1.Compute();
      res = pid1.getOutput();

      if (res >= 0)
        gpio_set_level(PinDIR, 0);
      else
        gpio_set_level(PinDIR, 1);

      ledcWrite(0, 1023 - fabs(res / 100.0) * 1023);
    }
    else
    {
      res = 0;
      ledcWrite(0, 1023);
    }

    /*int voltage = analogReadMilliVolts(PinISENS);

    float i = (voltage - 301) * 11.0; // 300mV offset is 3.3V * 10k/(10k+100K) and 1.1 factor is 100K/(10K+100K) * 1/0.1 Ohm
    if (i < 0)
      i = 0;
    Isens = Isens * 0.5 + i * 0.5;*/

#endif

    outputGUI = res;
    delay(20);
  }
}

void loopISens(void *param)
{
  float V_offset = 300; //  300mV offset is 3.3V * 10k/(10k+100K)
  float measure;

  const uint32_t N_Sample = 50;     // Average over N_Sample samples for fast lock
  const uint32_t N_Sample_HR = 500; // Average over N_Sample_HR samples for long term regulation
  const float RSENS = 0.10;         // 100mOhm Rsens

  while (true)
  {
    // Make 3 measurments and keep the median
    float a = analogReadMilliVolts(PinISENS);
    // delay(1);
    float b = analogReadMilliVolts(PinISENS);
    // delay(1);
    float c = analogReadMilliVolts(PinISENS);
    // delay(1);

    measure = mediane(a, b, c);

    if (!outputEnable)
    { // refresh the offset
      V_offset = 0.9 * V_offset + 0.1 * measure;
    }

    float Imeas = (measure - V_offset) * 11.0; // 300mV offset is 3.3V * 10k/(10k+100K) and 1.1 factor is 100K/(10K+100K) * 1/0.1 Ohm

    if (Imeas < 0)
      Imeas = 0;
    Isens = ((N_Sample - 1) * Isens + Imeas) / N_Sample;
    // isens = measure;
    // isens_HR = ((N_Sample_HR - 1) * isens_HR + Imeas) / N_Sample_HR;

    if (!Detecteur_AUX)
    {

      a = analogReadMilliVolts(PinInput);
      // delay(1);
      b = analogReadMilliVolts(PinInput);
      // delay(1);
      c = analogReadMilliVolts(PinInput);
      // delay(1);

      measure = mediane(a, b, c);

      VInput = (measure/1000.0 - 1.443) * 7.060606060606061; // coeff calculer dans etage-entree.py

      xact = VInput;
      turn = VInput / 20;
    }

    delay(1);
  }
}
