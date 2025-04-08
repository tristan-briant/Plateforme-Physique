#include <M5Unified.h>
#include "PID_v1.h"
#include "button.h"

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
  ledcWrite(1, 0);
  gpio_set_level(PinBL, 1);

  // xQueue = xQueueCreate(1, sizeof(double));
  xTaskCreatePinnedToCore(loopGUI, NULL, 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopServo, "", 10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "", 10000, NULL, 0, NULL, 0);
  //xTaskCreatePinnedToCore(loopSignal, "", 10000, NULL, 0, NULL, 0);

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
//{}

//void loopSignal(void *param)
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

      // outputGUI=output;
      delayMicroseconds(10);
    }
  }
}

void loopGUI(void *param)
{
  float I_old, Q_old;
  static float angle_old = 0;
  float angle_out = 0, angle_out_old = 0;
  const int x0 = 200, y0 = 130;
  const int r = 100;
  int r1, r2;
  float cosin, sinus;

  r1 = 80, r2 = 85;
  const int N_TICKS = 10;

  M5Canvas img(&M5.Lcd);
  img.createSprite(320, 40); // Create a 320x240 canvas

  M5Canvas img2(&M5.Lcd);
  img2.createSprite(100, 100); // Create a 320x240 canvas

  Button buttonOnOFF(0, 0, 320, 200);

  for (int i = 0; i < N_TICKS; i++)
  {
    cosin = cos(2 * PI * i / N_TICKS); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(2 * PI * i / N_TICKS); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, WHITE);
  }

  while (true)
  {
    M5.update();
    if (M5.BtnA.isPressed())
    {
      turn = 0;
      turn_frac0 = turn_frac;
      turnTotal_avg = turn_frac;
    }
    if (buttonOnOFF.wasPressed())
    {
      outputEnable = !outputEnable;

      if (outputEnable)
        pid1.SetMode(AUTOMATIC);
      else
        pid1.SetMode(MANUAL);
    }

    float angle = (turnTotal_avg - turn_frac0) * 2 * PI * 4.5;
    angle_out = 0.5 * angle_out + 0.5 * outputGUI * PI / 100.0;

    // double out;
    // angle_out = output * PI;

    r1 = 25, r2 = 45;
    cosin = cos(angle_out_old); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle_out_old); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, BLACK);
    cosin = cos(angle_out); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle_out);
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, YELLOW);

    r1 = 50, r2 = 75;
    cosin = cos(angle_old / 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle_old / 10); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, BLACK);
    cosin = cos(angle / 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle / 10);
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, GREEN);

    /*r1 = 10, r2 = 20;
    cosin = cos(angle_old * 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle_old * 10); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, BLACK);
    cosin = cos(angle * 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle * 10);
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, CYAN);
*/

    img.fillSprite(BLACK);
    char str[80];
    // sprintf(str, "%10.2f", (turnTotal_avg - turn_frac0) * 4.25);
    img.setTextColor(WHITE);
    img.setTextDatum(BL_DATUM);
    img.setFont(&FreeMono18pt7b);

    /* int x = abs((turnTotal_avg - turn_frac0) * 4.25 * 1000);
     int dec = (turnTotal_avg - turn_frac0) * 4.25;
     int frac1 = */

    xact = -(turnTotal_avg - turn_frac0) * 4.25;

    sprintf(str, "%8.3f mm", xact);
    img.drawString(str, 50, 30);

    img2.fillSprite(DARKGREY);
    img2.setTextColor(WHITE);
    img2.setTextDatum(CL_DATUM);
    img2.setFont(&FreeMonoBold9pt7b);
    sprintf(str, "P:%5.3f", pid1.GetKp());
    img2.drawString(str, 5, 15);
    sprintf(str, "I:%5.3f", pid1.GetKi());
    img2.drawString(str, 5, 30);
    sprintf(str, "D:%5.3f", pid1.GetKd());
    img2.drawString(str, 5, 45);
    sprintf(str, "Max:%3.0f%%", pid1.GetOutMax());
    img2.drawString(str, 5, 60);
    sprintf(str, "Min:%3.0f%%", pid1.GetOutMin());
    img2.drawString(str, 5, 75);
    outputGUI=pid1.getOutput();
    sprintf(str, "OUT:%3.0f%%", pid1.getOutput());
    if (outputEnable)
      img2.setTextColor(ORANGE);
    else
      img2.setTextColor(TFT_SILVER);
    img2.drawString(str, 5, 92);

    img.pushSprite(0, 0);
    img2.pushSprite(0, 40);
    angle_old = angle;
    angle_out_old = angle_out;

    M5.Lcd.setCursor(0, 220);
    M5.Lcd.printf("%4d", t_loop_us);

    M5.Lcd.setCursor(120, 220);
    M5.Lcd.printf("%6.2f", outputGUI);

    delay(20);
  }
}

void loopServo(void *param)
{

  while (true)
  {

    if (pid1.Compute())
    {
      res = pid1.getOutput();

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
    delay(2);
  }
}
