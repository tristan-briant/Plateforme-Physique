#include "M5Dial.h"
#include "Encoder.h"
#include "driver/ledc.h"
#include "M5Unified.h"
#include "button.h"
#include "driver/adc.h"
#include <EEPROM.h>

bool TEST_PWM = false;
long t_test;

///////  OutPut PWM  //////////
int PWMPin = GPIO_NUM_2;
int ISensPin = GPIO_NUM_13;
int ISensAuxPin = GPIO_NUM_1;
int VSensPin = GPIO_NUM_15;

const float RSENS = 0.5;

const int N_PWM = 110;
float PWM[N_PWM];
#define EEPROM_SIZE N_PWM * sizeof(float)

M5Canvas img(&M5.Lcd);
Button buttonScreen(0, 0, 240, 240);
double duty_cycle = 0;
enum Mode
{
  LIGHT_ON,
  LIGHT_OFF,
  LIGHT_TIMER
};

Mode mode = LIGHT_ON;

bool locked = false;

char str[100];

float measure = 0;

const int freqPWM = 78000;
double isens, isens_HR;
float IMAX = 1500;                // Imax (mA)
float PMAX = 100;                 // Max Power (%)
int PWM_MAX = 350;                // duty cycle max
double gain = PWM_MAX * 1e-6;     // Fast Gain achieve PWM_MAX in 1s
double gain_HR = PWM_MAX * 10e-9; // HR Gain achieve PWM_MAX in 100s

const u32_t N_Sample = 50;     // Average over N_Sample samples for fast lock
const u32_t N_Sample_HR = 500; // Average over N_Sample_HR samples for long term regulation

const int NbitPWM = 9;
const int MaxPWM = (1 << NbitPWM) - 1;

double iset = 0;

// void loopMesure(void *param);
void loopGUI(void *param);

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
  return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3;
}

uint16_t color565(uint8_t grey)
{
  return (grey & 0xF8) << 8 | (grey & 0xFC) << 3 | grey >> 3;
}

void IRAM_ATTR Ext_INT1_ISR()
{
  static int count = 0;
  static int x;
  count++;

  if (count == 10)
  {

    delayMicroseconds(10);
    // x = ((N_Sample - 1) * x) / N_Sample + analogReadMilliVolts(ISensPin);
    // measure = analogReadMilliVolts(ISensPin);
    measure = analogRead(ISensPin);
    count = 0;

    // const float alpha = 0.01;
    // isens = (1 - alpha) * isens + alpha * (measure * 10);
  }
}

void setup()
{
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5Dial.begin(cfg, true, false);

  Serial1.begin(115200);
  M5.begin();
  Serial.begin(115200);
  Serial.println("start");
  M5Dial.Speaker.setVolume(30);
  init_encoder();

  Serial.println(sizeof(char));
  Serial.println(sizeof(int));

  Serial.println(sizeof(long long));
  millis();

  EEPROM.begin(EEPROM_SIZE);

  for (int i = 0; i < N_PWM; i++)
  {
    float p = EEPROM.readFloat(i * sizeof(float));
    if (p <= PWM_MAX)
      PWM[i] = p;
  }

  pinMode(ISensPin, INPUT);
  analogSetPinAttenuation(ISensPin, ADC_0db);

  pinMode(ISensAuxPin, INPUT);
  analogSetPinAttenuation(ISensAuxPin, ADC_0db);

  pinMode(VSensPin, OUTPUT);
  analogSetPinAttenuation(VSensPin, ADC_11db);
  analogSetClockDiv(255);

  // analogSetPinAttenuation(VSensPin, ADC_0db);

  /*ledc_timer_config_t ledc_conf;
  ledc_conf.speed_mode = LEDC_SPEED_MODE_MAX;
  ledc_conf.freq_hz = LEDC_APB_CLK_HZ;
  ledc_conf.clk_cfg = LEDC_USE_APB_CLK;
  ledc_conf.timer_num = LEDC_TIMER_0;
  ledc_timer_config(&ledc_conf);*/
  ledcSetup(0, freqPWM, NbitPWM);

  ledcAttachPin(PWMPin, 0);
  // attachInterrupt(PWMPin, Ext_INT1_ISR, RISING);

  // xTaskCreatePinnedToCore(loopMesure, "", 2000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(loopGUI, "", 2000, NULL, 0, NULL, 0);

  img.createSprite(240, 240);
}

long oldPosition = -999;

float x;
float power = 0;

void loopGUI(void *) // on core 1
{
  while (true)
  {
    static long t_old = 0;
    long t = micros();

    M5Dial.update();
    long x = get_encoder();
    clear_encoder();

    if (x != 0)
    {
      /*if (x > 0)
        M5Dial.Speaker.tone(6000, 20);
      else
        M5Dial.Speaker.tone(5000, 20);*/
      power = constrain(power + x, 0, MaxPWM);
      locked = false;
    }

    if (M5Dial.BtnA.wasPressed())
    {
      if (power != 0)
        power = 0;
      else
        power = 100;
      locked = false;
    }

    if (Serial.available())
    {
      if (Serial.read() == 't')
        TEST_PWM = true;
      power = 0;
    }

    if (buttonScreen.wasPressed())
    {
      if (mode == LIGHT_OFF)
        mode = LIGHT_ON;
      else if (mode == LIGHT_ON)
        mode = LIGHT_OFF;
    }
    if (t - t_old > 100000)
    {
      img.fillSprite(BLACK);

      ///////////// Le pourcentage /////////////////
      if (mode == LIGHT_OFF)
        img.setTextColor(color565(128, 90, 0));
      else
        img.setTextColor(ORANGE);

      img.setTextDatum(middle_center);
      img.setFont(&fonts::DejaVu72);
      img.setTextSize(1);
      img.drawString(String((int)power) + "%",
                     M5Dial.Display.width() / 2,
                     M5Dial.Display.height() / 2);

      float voltage = analogReadMilliVolts(VSensPin) * 11.0 / 1000;
      img.setTextColor(color565(50));
      img.setTextDatum(middle_center);
      img.setFont(&fonts::DejaVu24);
      img.setTextSize(1);

      static double isens_print, voltage_print;

      if (locked)
        isens_print = isens_HR;
      else
        isens_print = isens;
      // isens_print = analogReadMilliVolts(ISensAuxPin);

      img.setTextColor(color565(50));

      if (fabs(isens_print - isens_HR) > 1.0)
        isens_print = isens_HR;

      if (fabs(voltage_print - voltage) > 0.5)
        voltage_print = voltage;
    
          // voltage_print = (float)t / 1e6;
          // isens_print = (float)t_test / 1e6;

          sprintf(str, "%d.%03dA   %2d.%01dV", (int)(isens_print / 1000), ((int)isens_print % 1000), (int)voltage_print, ((int)(voltage_print * 10) % 10));
      t_old = t;
      // Serial.println(duty_cycle, 1);

      img.drawString(String(str),
                     M5Dial.Display.width() / 2,
                     M5Dial.Display.height() * 3 / 4);

      sprintf(str, "%dmA  ", (int)(iset));
      img.setTextColor(locked ? color565(0, 128, 0) : color565(50));
      img.drawString(String(str),
                     M5Dial.Display.width() / 2,
                     M5Dial.Display.height() * 3 / 4 + 20);

      img.pushSprite(0, 0);

      if (TEST_PWM)
      {
        static bool first_time = true;
        if (first_time)
        {
          Serial.println("power,i,v,pwm");
          first_time = false;
        }
        if (locked)
        {
          Serial.print(power);
          Serial.print(",");
          Serial.print(isens_HR);
          Serial.print(",");
          Serial.print(voltage);
          Serial.print(",");
          Serial.println(duty_cycle);

          PWM[(int)power] = duty_cycle;
          locked = false;
          power += 1;
          if (power >= N_PWM)
          {

            for (int i = 0; i < N_PWM; i++)
            {
              EEPROM.writeFloat(i * sizeof(float), PWM[i]);
            }
            EEPROM.commit();
            TEST_PWM = false;
            power = 0;
          }
        }
      }
    }
    /*Serial.print(voltage, 1);
    Serial.print("    ");
    Serial.println(isens, 0);*/
    delay(10);
  }
}

const double alpha = 0.1;

// attachInterrupt(PWMPin, Ext_INT1_ISR, RISING); // on core 0

unsigned long t_old = 0;

u32_t count = 0;      // pour moduler le PWM avec la partie décimale du duty_cycle
const u32_t PDEC = 8; // precision décimale

void loop()
// void loopMesure(void *param)
{
  t_test = micros();
  // while (true)
  {

    delayMicroseconds(1);
    // gpio_set_level((gpio_num_t)VSensPin, 1);
    // int x;
    // adc2_get_raw(adc2_channel_t::ADC2_CHANNEL_5, adc_bits_width_t::ADC_WIDTH_MAX, &x);
    // measure = x;
    measure = analogReadMilliVolts(ISensPin);
    // gpio_set_level((gpio_num_t)VSensPin, 0);

    // measure= analogRead(ISensPin);

    float Imeas = measure / RSENS; // analogReadMilliVolts(ISensPin) * 10;

    isens = ((N_Sample - 1) * isens + Imeas) / N_Sample;
    isens_HR = ((N_Sample_HR - 1) * isens_HR + Imeas) / N_Sample_HR;

    // isens=isens_HR=measure;

    /*x = ((N_Sample - 1) * x) / N_Sample + analogReadMilliVolts(ISensPin);
    measure = x;
*/
    // if (duty_cycle == 0) // Plus de mesure par interrupt on le fait manuellement
    //   Ext_INT1_ISR();

    // isens = (float)measure / N_Sample * 10;

    unsigned long t_new = micros();
    if (t_new < t_old || (t_old > t_old + 200UL)) // overflow
    {
      Serial.println("overflow");
      Serial.println(t_old);
      Serial.println(t_new);

      Serial.println();
      t_old = t_new;
    }

    if (t_new > t_old + 200UL)
    {
      switch (mode)
      {
      case LIGHT_OFF:
        iset = 0;
        break;
      case LIGHT_ON:
        iset = power * IMAX / 100;
        break;
      }

      /* if (isens < iset - 50) //&& duty_cycle < PWM_MAX * 0.9)
       {
         duty_cycle += gain * (t_new - t_old);
       }*/
      /*else if (isens_HR < iset - 5)
      {
        duty_cycle += gain_HR * (t_new - t_old);
      }*/
      static int countdown = 0;

      float epsilon = constrain(iset / 10, 20, 40);

      int delta_t = (int)(t_new - t_old);

      if (fabs(isens - iset) >= epsilon && !locked)
      {

        // if (isens < iset)
        duty_cycle += gain * delta_t * constrain((iset - isens) / 150, -20, 10);
        // else
        //   duty_cycle -= gain * (t_new - t_old);
      }
      // else if (isens_HR > iset + 5)

      if (fabs(isens_HR - iset) >= epsilon / 2)
      {
        locked = false;
        countdown = 0;
      }

      if (fabs(isens - iset) < epsilon && !locked)
      {
        if (fabs(isens_HR - iset) < epsilon / 20)
        {
          countdown++;
          if (countdown == 100)
          {
            locked = true;

            countdown = 0;
          }
        }
        else
          countdown = 0;

        // if (iset > 100)
        duty_cycle += gain_HR * delta_t * constrain(iset - isens_HR, -100, 100);
        // else
        //   duty_cycle -= gain_HR * (t_new - t_old) * (isens_HR - iset) / 10.0;
      }

      // if (TEST_PWM)
      duty_cycle = constrain(duty_cycle, 0, PWM_MAX);
      // else
      //  duty_cycle = power;
      // duty_cycle = PWM[constrain((int)power, 0, N_PWM - 1)];

      /*float dec = (duty_cycle - floorf(duty_cycle)) * PDEC;
      if (dec > count)
        ledcWrite(0, duty_cycle + 1);
      else*/
      ledcWrite(0, duty_cycle);

      count = (count + 1) % PDEC;
      t_old = t_new;
    }
  }
}