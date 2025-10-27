#include <M5Unified.h>
#include <M5dial.h>
#include "PID_v1.h"
#include <DS18B20.h>
#include "Encoder.h"
#include "button.h"

// #define DEBUG

char str[100];

///////  OutPut Pin  //////////

// PORT A
gpio_num_t Pin_DIR = GPIO_NUM_13;
gpio_num_t Pin_PWM = GPIO_NUM_15;

// PORT B
gpio_num_t ISensPin = GPIO_NUM_2;
DS18B20 ds(GPIO_NUM_1);

const int freqPWM = 100000;
const int MaxPWM = 255;
const int NbitPWM = 8;

bool outputEnable = true;

/// PID variables //////
double temp1, temp2, tset1 = 20, tset2 = 20;
double output1;

const double T_factor = 143.0 / 4095.0;

double isens, isens_HR, V_offset;
float measure = 0;

PID pid1(&temp1, &output1, &tset1, 0, 0, 0, DIRECT);

///////////////// Function declaration ///////////////
void loopComunication(void *param);
bool load_value(const char *name, const char *key, double *target);
void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1);
void load_tuning(PID *pid1);

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
  return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3;
}

uint16_t color565(uint8_t grey)
{
  return (grey & 0xF8) << 8 | (grey & 0xFC) << 3 | grey >> 3;
}
///////////////// Function declaration ///////////////

void loopGUI(void *param)
{
  M5Canvas img(&M5.Lcd);
  Button buttonScreen(0, 0, 240, 240);

  img.createSprite(240, 240);
  int page = 0;

  while (true)
  {
    M5.update();

    img.fillScreen(BLACK);
    if (page == 0) // MAIN
    {

      int x0 = 120, y0 = 70, r0 = 70;
      img.fillCircle(x0, y0, r0, pid1.GetMode() ? ORANGE : DARKGREY);
      // img.setFont(&FreeSansBold24pt7b);
      // img.setTextSize(2);
      img.setTextColor(pid1.GetMode() ? WHITE : LIGHTGREY);
      img.setFont(&fonts::FreeSansBold24pt7b);

      img.setTextDatum(MC_DATUM);
      sprintf(str, "%.1f", temp1);
      img.drawString(str, x0, y0 + 5);

      x0 = 180, y0 = 145, r0 = 35;
      img.fillCircle(x0, y0, r0, DARKCYAN);
      img.setTextSize(1);
      // img.setTextColor(pid1.GetMode() ? GREEN : WHITE);
      // img.setTextColor(pid1.GetMode() ? GREEN : WHITE);

      img.setFont(&FreeSansBold12pt7b);
      img.setTextDatum(MC_DATUM);
      sprintf(str, "%.1f", tset1);
      img.drawString(str, x0, y0 + 5);

      img.setTextColor(color565(50));

      img.setTextColor(color565(50));
      img.setTextDatum(middle_center);
      img.setFont(&fonts::DejaVu18);
      img.setTextSize(1);
      x0 = M5Dial.Display.width() / 2;
      y0 = 205;

      if (pid1.GetMode() == 0)
      {
        img.drawString("OFF", x0, y0);
      }
      else
      {
        if (output1 < 0)
        {
          img.drawString("COOLING", x0, y0);
          sprintf(str, "%.0f%%", 100 * fabs(output1 / pid1.GetOutMin()));
          img.drawString(str, x0, y0 + 20);
        }
        else
        {
          img.drawString("HEATING", x0, y0);
          sprintf(str, "%.0f%%", 100 * fabs(output1 / pid1.GetOutMax()));
          img.drawString(str, x0, y0 + 20);
        }
      }
    }
    else if (page == 1) // Option
    {
      img.setTextDatum(MC_DATUM);
      img.fillScreen(BLACK);

      img.setFont(&FreeMonoBold12pt7b);

      img.setCursor(0, 90);
      img.printf("%5.1f Set: %3.1f", temp1, tset1);
      img.setTextSize(1);
      int k = 110;

      img.setFont(&FreeMonoBold12pt7b);

      img.setCursor(0, k);
      img.printf("%4.0f%%<%4.0f%%<%4.0f%%", pid1.GetOutMin(), output1, pid1.GetOutMax());
      k += 20;
      img.setCursor(35, k);
      img.printf("GP: %7.2g", pid1.GetKp());
      k += 20;
      img.setCursor(35, k);
      img.printf("GI: %7.2g", pid1.GetKi());
      k += 20;
      img.setCursor(35, k);
      img.printf("GD: %7.2g", pid1.GetKd());
      k += 20;
      img.setCursor(35, k);
      static float i_affiche = 0;
      // if (isens_HR >= i_affiche + 5 || isens_HR <= i_affiche - 5)
      // i_affiche = 0.001 * ((int)(fabs(isens_HR) / 10));
      img.printf(" I = %4.2f A", fabs(isens_HR) / 1000);

      k += 20;
      img.setCursor(35, k);
      float i_peltier = 0;
      if (fabs(output1) >= 4)
        i_peltier = fabs(isens_HR / pow(fabs(output1) / 95.0, 0.8) / 1000);
      img.printf(" I = %4.2f A", i_peltier);
    }
    /*
        img.setTextColor(PINK, BLACK);
        img.setCursor(0, 100);
        img.printf("  T2 = %7.2fC", temp2);

        img.setCursor(0, 130);
        img.printf("Set2 = %7.2fC", tset2);

        img.setCursor(0, 160);
        img.printf("Out2 = %7.1f%", output2);
    */
    /// Handle Butons

    /* float inc= 0;

     if (get_encoder() >= 4)
     {
       inc = 0.5;
       clear_encoder();
     }
     if (get_encoder() <= -4)
     {
       inc = -0.5;
       clear_encoder();
     }*/

#ifdef DEBUG
    output1 += get_encoder();
    output1 = constrain(output1, -100, 100);
#else
    tset1 += get_encoder() * 0.125f;
#endif

    clear_encoder();

    if (M5.BtnA.wasPressed())
    {
      page++;
      if (page == 2)
        page = 0;
    }

    if (buttonScreen.wasPressed())
    {
      outputEnable = !outputEnable;
      pid1.SetMode(outputEnable);
    }

    /*if (M5.BtnB.wasPressed() || M5.BtnB.pressedFor(200))
      tset1 -= 0.1;
    if (M5.BtnC.wasPressed() || M5.BtnC.pressedFor(200))
      tset1 += 0.1;

    if (M5.BtnB.wasReleased() || M5.BtnC.wasReleased())
      save_value("PID1", "TSET", tset1);*/

    img.pushSprite(0, 0);
    delay(100);
  }
}

void loopServo(void *param)
{
  const double alpha = 0.001; // smooth temp measurment

  // temp1 = analogRead(35) * T_factor;

  while (true)
  {
    // double x = analogRead(35) * T_factor;
    // temp1 = alpha * x + (1 - alpha) * temp1;

    temp1 = ds.getTempC();

    // x = analogRead(36) * T_factor;
    // temp2 = alpha * x + (1 - alpha) * temp2;

#ifndef DEBUG
    pid1.Compute();
#endif
    // pid2.Compute();

    if (outputEnable)
    {
      if (output1 >= 0)
      {
        // ledcWrite(0, 0);
        gpio_set_level(Pin_DIR, 0);
      }
      else
      {
        gpio_set_level(Pin_DIR, 1);
        // ledcWrite(0, 1);
      }
      ledcWrite(0, MaxPWM * (1 - fabs(output1 / 100.0)));
    }
    else
    {
      // ledcWrite(0, 0);
      ledcWrite(0, MaxPWM);
    }

    delay(100);
  }
}

void setup()
{
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5Dial.begin(cfg, true, false);
  Serial.begin(115200);
  delay(100);
  Serial.println("Devices: ");
  Serial.println(ds.getNumberOfDevices());
  Serial.println();

  init_encoder();

  // M5.Lcd.setFont(&FreeMonoBold18pt7b);
  ds.setResolution(12);

  gpio_set_level(Pin_PWM, 1);
  pinMode(Pin_DIR, OUTPUT);
  pinMode(Pin_PWM, OUTPUT);

  ledcSetup(0, freqPWM, NbitPWM);
  // ledcSetup(1, freqPWM, NbitPWM);

  // ledcAttachPin(out1Pin[0], 0);
  // ledcAttachPin(Pin_DIR, 0);
  ledcAttachPin(Pin_PWM, 0);
  ledcWrite(0, MaxPWM);

  adcAttachPin(ISensPin);
  // pinMode(ISensPin, INPUT);
  //  analogSetPinAttenuation(ISensPin, ADC_0db);

  analogReadResolution(12);
  analogSetClockDiv(1);
  analogSetAttenuation(ADC_0db);

  pid1.SetOutputLimits(-100, 100);

  xTaskCreatePinnedToCore(loopServo, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(loopGUI, "", 4000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "", 4000, NULL, 1, NULL, 0);

  // Gd1 = load_value("PID1", "GP");
  load_tuning(&pid1);
  load_value("PID1", "TSET", &tset1);
  // load_value("PID2", "TSET", &tset2);
  double max = 100, min = -100;
  load_value("PID1", "OUTMAX", &max);
  load_value("PID1", "OUTMIN", &min);
  // pid1.SetOutputLimits(min, max);
  /*max = 100, min = -100;
  load_value("PID2", "OUTMAX", &max);
  load_value("PID2", "OUTMIN", &min);
  pid2.SetOutputLimits(min, max);*/

  Serial.println(pid1.GetKp());
  Serial.println(pid1.GetKi());
  Serial.println(pid1.GetKd());
  Serial.println(pid1.GetDirection());
  Serial.println(pid1.GetMode());

  pid1.SetMode(MANUAL);
  outputEnable = false;
  // pid2.SetMode(MANUAL);
}

void loop()
{

  const u32_t N_Sample = 50;     // Average over N_Sample samples for fast lock
  const u32_t N_Sample_HR = 500; // Average over N_Sample_HR samples for long term regulation
  const float RSENS = 0.10;      // 100mOhm Rsens

  // Make 3 measurments and keep the median
  float a = analogReadMilliVolts(ISensPin);
  float b = analogReadMilliVolts(ISensPin);
  float c = analogReadMilliVolts(ISensPin);
  float x;

  if (a > b)
  {
    x = a;
    a = b;
    b = x;
  }
  if (c > b)
    measure = b;
  else if (c > a)
    measure = c;
  else
    measure = a;

  if (!outputEnable)
  { // refresh the offset
    V_offset = 0.9 * V_offset + 0.1 * measure;
  }

  float Imeas = (measure - V_offset) / RSENS;

  isens = ((N_Sample - 1) * isens + Imeas) / N_Sample;
  // isens = measure;
  isens_HR = ((N_Sample_HR - 1) * isens_HR + Imeas) / N_Sample_HR;

  /*Serial.print(measure);
  Serial.print(',');
  Serial.println(isens);*/
  delayMicroseconds(1);
}
