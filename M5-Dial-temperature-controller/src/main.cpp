#include <M5Unified.h>
#include <M5dial.h>
#include "PID_v1.h"
#include <DS18B20.h>
#include "Encoder.h"

char str[100];

///////  OutPut Pin  //////////

gpio_num_t Pin_PWM1 = GPIO_NUM_13;
gpio_num_t Pin_PWM2 = GPIO_NUM_15;
gpio_num_t ISensPin = GPIO_NUM_2;
DS18B20 ds(GPIO_NUM_1);

const int freqPWM = 20;
// const int MaxPWM = 4095;
const int NbitPWM = 11;

bool outputEnable = true;

/// PID variables //////
double temp1, temp2, tset1 = 20, tset2 = 20;
double output1, output2;

const double T_factor = 143.0 / 4095.0;

double isens, isens_HR;
float measure = 0;

// int pid_enable = 1;

PID pid1(&temp1, &output1, &tset1, 0, 0, 0, DIRECT);
// PID pid2(&temp2, &output2, &tset2, 0, 0, 0, DIRECT);

///////////////// Function declaration ///////////////
void loopComunication(void *param);
bool load_value(const char *name, const char *key, double *target);
void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1);  //, PID pid2);
void load_tuning(PID *pid1); //, PID *pid2);
///////////////// \Function declaration ///////////////

void loopGUI(void *param)
{

  while (true)
  {
    M5.update();
    /*int x0 = 120, y0 = 50, r0 = 60;
    M5.Lcd.fillCircle(x0, y0, r0, DARKCYAN);
    x0 = 180, y0 = 120, r0 = 40;
    M5.Lcd.fillCircle(x0, y0, r0, DARKGREY);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(pid1.GetMode() ? GREEN : WHITE);

    x0 = 120, y0 = 50, r0 = 60;
    M5.Lcd.setFont(&FreeSansBold24pt7b);
    M5.Lcd.setTextDatum(MR_DATUM);
    sprintf(str, "%.1f", temp1);
    M5.Lcd.drawString(str, x0 + 40, y0 + 5);

    x0 = 180, y0 = 120, r0 = 40;
    M5.Lcd.setTextColor(pid1.GetMode() ? GREEN : WHITE);

    M5.Lcd.setFont(&FreeSansBold18pt7b);
    M5.Lcd.setTextDatum(MC_DATUM);
    sprintf(str, "%.1f", tset1);
    M5.Lcd.drawString(str, x0, y0 + 5);
*/
    // M5.Lcd.setFont(&FreeSansBold18pt7b);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.fillScreen(BLACK);

    M5.Lcd.setFont(&FreeMonoBold12pt7b);

    M5.Lcd.setCursor(0, 90);
    M5.Lcd.printf("%5.1f Set: %3.1f", temp1, tset1);
    M5.Lcd.setTextSize(1);
    int k = 130;

    M5.Lcd.setFont(&FreeMonoBold12pt7b);

    M5.Lcd.setCursor(0, k);
    M5.Lcd.printf("%4.0f%%<%4.0f%%<%4.0f%%", pid1.GetOutMin(), output1, pid1.GetOutMax());
    k += 20;
    M5.Lcd.setCursor(35, k);
    M5.Lcd.printf("GP: %7.2g", pid1.GetKp());
    k += 20;
    M5.Lcd.setCursor(35, k);
    M5.Lcd.printf("GI: %7.2g", pid1.GetKi());
    k += 20;
    M5.Lcd.setCursor(35, k);
    M5.Lcd.printf("GD: %7.2g", pid1.GetKd());
    k += 20;
    M5.Lcd.setCursor(35, k);
    static float i_affiche = 0;
    if (isens_HR >= i_affiche + 5 || isens_HR <= i_affiche - 5)
      i_affiche = 0.001 * ((int)(isens_HR / 10));
    M5.Lcd.printf(" I = %4.2f A", isens_HR/1000);

    /*
        M5.Lcd.setTextColor(PINK, BLACK);
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.printf("  T2 = %7.2fC", temp2);

        M5.Lcd.setCursor(0, 130);
        M5.Lcd.printf("Set2 = %7.2fC", tset2);

        M5.Lcd.setCursor(0, 160);
        M5.Lcd.printf("Out2 = %7.1f%", output2);
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

    tset1 += get_encoder() * 0.1f;
    clear_encoder();

    if (M5.BtnA.wasPressed())
      pid1.SetMode(!pid1.GetMode()); //

    /*if (M5.BtnB.wasPressed() || M5.BtnB.pressedFor(200))
      tset1 -= 0.1;
    if (M5.BtnC.wasPressed() || M5.BtnC.pressedFor(200))
      tset1 += 0.1;

    if (M5.BtnB.wasReleased() || M5.BtnC.wasReleased())
      save_value("PID1", "TSET", tset1);*/

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

    pid1.Compute();
    // pid2.Compute();

    if (outputEnable)
    {
      if (output1 >= 0)
      {
        ledcWrite(0, fabs(output1 / 100.0) * 2047);
        ledcWrite(1, 0);
      }
      else
      {
        ledcWrite(0, 0);
        ledcWrite(1, fabs(output1 / 100.0) * 2047);
      }
    }
    else
    {
      ledcWrite(0, 0);
      ledcWrite(1, 0);
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

  M5.Lcd.setFont(&FreeMonoBold18pt7b);
  ds.setResolution(12);

  pinMode(Pin_PWM1, OUTPUT);
  pinMode(Pin_PWM2, OUTPUT);

  ledcSetup(0, freqPWM, NbitPWM);
  ledcSetup(1, freqPWM, NbitPWM);

  // ledcAttachPin(out1Pin[0], 0);
  ledcAttachPin(Pin_PWM1, 0);
  ledcAttachPin(Pin_PWM2, 1);

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
  // pid2.SetMode(MANUAL);
}

void loop() 
{

  const u32_t N_Sample = 50;     // Average over N_Sample samples for fast lock
  const u32_t N_Sample_HR = 500; // Average over N_Sample_HR samples for long term regulation
  const float RSENS = 0.1;       // 100mOhm Rsens


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

  // measure = analogReadMilliVolts(ISensPin);
  // measure = analogRead(ISensPin);

  float Imeas = measure / RSENS;

  isens = ((N_Sample - 1) * isens + Imeas) / N_Sample;
  // isens = measure;
  isens_HR = ((N_Sample_HR - 1) * isens_HR + Imeas) / N_Sample_HR;

  /*Serial.print(measure);
  Serial.print(',');
  Serial.println(isens);*/
  delayMicroseconds(1);
}
