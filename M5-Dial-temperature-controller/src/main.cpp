#define LMT01_SENSOR
// #define DS18B20_SENSOR

#include <M5Unified.h>
#include <M5dial.h>
#include "PID_v1.h"
#include "Encoder.h"
#include "button.h"

#ifdef DS18B20_SENSOR
#include <DS18B20.h>
#endif

#ifdef LMT01_SENSOR
#include "LMT01.h"
#endif

// #define DEBUG

char str[100];

int ScreenRotation = 0;

///////  OutPut Pin  //////////

// PORT A
gpio_num_t Pin_DIR = GPIO_NUM_13;
gpio_num_t Pin_PWM = GPIO_NUM_15;

// PORT B
gpio_num_t ISensPin = GPIO_NUM_2;
gpio_num_t TempSensPin = GPIO_NUM_1;

#ifdef DS18B20_SENSOR
DS18B20 ds(TempSensPin);
#endif

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
void loopISens(void *param);
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

  M5.Lcd.setRotation(ScreenRotation);
  img.createSprite(240, 240);
  int page = 0;
  int selection = 0;
  bool unlock = false;

  while (true)
  {
    M5.update();
    img.fillScreen(BLACK);

    if (page == 0) /////////////////////////////////// MAIN //////////////////////////////////////////
    {
#ifdef DEBUG
      output1 += get_encoder();
      output1 = constrain(output1, -100, 100);
#else
      tset1 += get_encoder() * 0.125f;
#endif
      clear_encoder();

      if (buttonScreen.wasPressed())
      {
        outputEnable = !outputEnable;
        pid1.SetMode(outputEnable);
      }

      int x0 = 120, y0 = 70, r0 = 70;
      img.fillCircle(x0, y0, r0, pid1.GetMode() ? ORANGE : DARKGREY);
      img.setTextColor(pid1.GetMode() ? WHITE : LIGHTGREY);
      img.setFont(&fonts::FreeSansBold24pt7b);

      img.setTextDatum(MC_DATUM);
      sprintf(str, "%.1f", temp1);
      img.drawString(str, x0, y0 + 5);

      x0 = 180, y0 = 145, r0 = 35;
      img.fillCircle(x0, y0, r0, DARKCYAN);
      img.setTextSize(1);

      img.setFont(&FreeSansBold12pt7b);
      img.setTextDatum(MC_DATUM);
      sprintf(str, "%.1f", tset1);
      img.drawString(str, x0, y0 + 5);

      img.setTextColor(color565(100));
      img.setTextDatum(middle_center);
      img.setFont(&fonts::DejaVu18);
      img.setTextSize(1);
      x0 = M5Dial.Display.width() / 2;
      y0 = 205;

      if (pid1.GetMode() == 0)
      {
        img.drawString("Touch the screen", x0, y0 - 5);
        img.drawString("to turn ON", x0, y0 + 15);
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
    else if (page == 1) ////////////////////////////// Option //////////////////////////////////////
    {

      if (unlock && buttonScreen.wasPressed())
        selection = (selection + 1) % 5;

      if (buttonScreen.pressedFor(1500))
      {
        unlock = true;
      }

      int a = get_encoder();
      clear_encoder();

      img.setTextDatum(MC_DATUM);
      img.fillScreen(BLACK);

      // img.setFont(&FreeMonoBold12pt7b);
      if (!unlock)
      {
        img.setFont(&fonts::DejaVu18);
        img.setTextColor(ORANGE);
        img.setCursor(0, 20);
        img.drawString("Long press", 120, 25);
        img.drawString("to unlock", 120, 50);
      }
      else
      {
        img.setFont(&fonts::DejaVu18);
        img.setTextColor(DARKGREEN);
        img.setCursor(0, 20);
        img.drawString("Touch", 120, 25);
        img.drawString("to select", 120, 50);
      }

      img.setTextColor(LIGHTGREY);
      img.setCursor(0, 90);
      img.printf("%5.1f Set: %3.1f", temp1, tset1);
      img.setTextSize(1);
      int k = 110;

      img.setFont(&FreeMonoBold12pt7b);

      if (selection == 0 && unlock)
      {
        img.fillRect(0, k - 10, 70, 20, DARKGREEN);
        float value = pid1.GetOutMin() + a;
        value = constrain(value, -100, 0);
        pid1.SetOutputLimits(value, pid1.GetOutMax());
      }

      if (selection == 1 && unlock)
      {
        img.fillRect(170, k - 10, 70, 20, DARKGREEN);
        float value = pid1.GetOutMax() + a;
        value = constrain(value, 0, 100);
        pid1.SetOutputLimits(pid1.GetOutMin(), value);
      }

      img.setCursor(0, k);
      img.printf("%4.0f%%<%4.0f%%<%4.0f%%", pid1.GetOutMin(), output1, pid1.GetOutMax());

      k += 20;
      if (selection == 2 && unlock)
      {
        img.fillRect(35 - 10, k - 10, 180, 20, DARKGREEN);
        float value = pid1.GetKp() + a * 0.1;
        value = constrain(value, 0, 100);
        pid1.SetTunings(value, pid1.GetKi(), pid1.GetKd());
        save_tuning(pid1);
      }
      img.setCursor(35, k);
      img.printf("GP: %7.1f", pid1.GetKp());

      k += 20;
      if (selection == 3 && unlock)
      {
        img.fillRect(35 - 10, k - 10, 180, 20, DARKGREEN);
        float value = pid1.GetKi() + a * 0.01;
        value = constrain(value, 0, 100);
        pid1.SetTunings(pid1.GetKp(), value, pid1.GetKd());
        save_tuning(pid1);
      }
      img.setCursor(35, k);
      img.printf("GI: %7.2f", pid1.GetKi());

      k += 20;
      if (selection == 4 && unlock)
      {
        img.fillRect(35 - 10, k - 10, 180, 20, DARKGREEN);
        float value = pid1.GetKd() + a * 0.1;
        value = constrain(value, 0, 100);
        pid1.SetTunings(pid1.GetKp(), pid1.GetKi(), value);
        save_tuning(pid1);
      }
      img.setCursor(35, k);
      img.printf("GD: %7.1f", pid1.GetKd());
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
    else if (page == 2)
    {
      img.setTextDatum(MC_DATUM);
      img.fillScreen(BLACK);
      img.setTextColor(ORANGE);
      img.setFont(&fonts::FreeSansBold18pt7b);
      img.drawString("Orientation", 120, 120);
      img.setTextColor(LIGHTGREY);
      img.setFont(&fonts::FreeSans12pt7b);
      img.drawString("Turn the knob", 120, 160);

      int a = get_encoder();
      if (abs(a) >= 4)
      {
        a = constrain(a, -1, 1);
        ScreenRotation = (ScreenRotation + a) % 4;
        if (ScreenRotation < 0)
          ScreenRotation = 3;
        M5.Lcd.setRotation(ScreenRotation);
        clear_encoder();
      }
    }

    if (M5.BtnA.wasPressed())
    {
      page++;
      if (page == 3)
        page = 0;

      clear_encoder();
      unlock = false;
      selection = 0;
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
#ifdef DS18B20_SENSOR
    temp1 = ds.getTempC();
#endif

#ifdef LMT01_SENSOR
    temp1 = get_temperature(TempSensPin);
#endif

#ifndef DEBUG
    pid1.Compute();
#endif

    if (outputEnable)
    {
      if (output1 >= 0)
      {
        gpio_set_level(Pin_DIR, 0);
      }
      else
      {
        gpio_set_level(Pin_DIR, 1);
      }
      ledcWrite(0, MaxPWM * (1 - fabs(output1 / 100.0)));
    }
    else
    {
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
#ifdef DS18B20_SENSOR
  Serial.println(ds.getNumberOfDevices());
#endif

#ifdef LMT01_SENSOR
  Serial.println("LMT01");
  init_LMT01(TempSensPin);
#endif

  Serial.println();

  init_encoder();

#ifdef DS18B20_SENSOR
  ds.setResolution(12);
#endif
  gpio_set_level(Pin_PWM, 1);
  pinMode(Pin_DIR, OUTPUT);
  pinMode(Pin_PWM, OUTPUT);

  ledcSetup(0, freqPWM, NbitPWM);

  ledcAttachPin(Pin_PWM, 0);
  ledcWrite(0, MaxPWM);

  adcAttachPin(ISensPin);

  analogReadResolution(12);
  analogSetClockDiv(1);
  analogSetAttenuation(ADC_0db);

  pid1.SetOutputLimits(-100, 100);

  xTaskCreatePinnedToCore(loopServo, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(loopGUI, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(loopComunication, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(loopISens, "", 4000, NULL, 1, NULL, 0);

  load_tuning(&pid1);
  load_value("PID1", "TSET", &tset1);
  double max = 100, min = -100;
  load_value("PID1", "OUTMAX", &max);
  load_value("PID1", "OUTMIN", &min);

  Serial.println(pid1.GetKp());
  Serial.println(pid1.GetKi());
  Serial.println(pid1.GetKd());
  Serial.println(pid1.GetDirection());
  Serial.println(pid1.GetMode());

  pid1.SetMode(MANUAL);
  outputEnable = false;
}

void loopISens(void *param)
{

  const u32_t N_Sample = 50;     // Average over N_Sample samples for fast lock
  const u32_t N_Sample_HR = 500; // Average over N_Sample_HR samples for long term regulation
  const float RSENS = 0.10;      // 100mOhm Rsens

  while (true)
  {
    // Make 3 measurments and keep the median
    float a = analogReadMilliVolts(ISensPin);
    // delay(1);
    float b = analogReadMilliVolts(ISensPin);
    // delay(1);
    float c = analogReadMilliVolts(ISensPin);
    // delay(1);
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

    // Serial.print(measure);
    // Serial.print(',');
    // Serial.println(isens);
    // delayMicroseconds(100);
    delay(1);
  }
}

void loop() {} // not used
