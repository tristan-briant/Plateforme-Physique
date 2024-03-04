#include <M5stack.h>

const int NThermo = 4;
const int PinThermo[] = {35, 36, 26, 2, 12, 13, 15, 0};
const int PinCommon = 34;

const int color[] = {GREEN, MAGENTA, CYAN, WHITE, WHITE, WHITE, WHITE, WHITE};

const double ADC2VOLT = 3.7 / 4095.0;
const double T_Factor = 75.0;

double Temperature[NThermo];
double common;

void TaskSensor(void *param);
void TaskGUI(void *param);

void setup()
{
  M5.begin();
  Serial.begin(115200);

  M5.Lcd.setTextSize(4);

  for (int i = 0; i < NThermo; i++)
  {
    pinMode(PinThermo[i], ANALOG);

    Temperature[i] = 0;
  }

  xTaskCreatePinnedToCore(TaskSensor, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskGUI, "", 4000, NULL, 1, NULL, 0);
}

void loop()
{
  /*double alpha = 0.00025;

  common = analogRead(PinCommon);
  for (int i = 0; i < NThermo; i++)
  {
    Temperature[i] = analogRead(PinThermo[i]);
  }

  while (true)
  {

    common = alpha * analogRead(PinCommon) + (1 - alpha) * common;

    for (int i = 0; i < NThermo; i++)
    {
      Temperature[i] = alpha * analogRead(PinThermo[i]) + (1 - alpha) * Temperature[i];
    }
  }*/
}

void TaskGUI(void *param)
{

  while (true)
  {
    for (int i = 0; i < NThermo; i++)
    {
      M5.Lcd.setCursor(0, i * 40);
      M5.Lcd.setTextColor(color[i], BLACK);

      M5.Lcd.printf("%7.2f C", (Temperature[i] - common) * ADC2VOLT * T_Factor);
    }

    if (Serial.available())
    {
      char str[80];
      Serial.setTimeout(1000);
      Serial.readBytes(str,2);

      //str = Serial.readString();
      //str.toUpperCase();

      if (str[0]=='T' && str[1]=='?')
      {
        for (int i = 0; i < NThermo; i++)
        {
          Serial.printf("%.3f", (Temperature[i] - common) * ADC2VOLT * T_Factor);
          if (i < NThermo - 1)
            Serial.print(",");
        }
        Serial.println();
      }
    }

    delay(10);
  }
}

void TaskSensor(void *param)
{
  double alpha = 0.00025;

  common = analogRead(PinCommon);
  for (int i = 0; i < NThermo; i++)
  {
    Temperature[i] = analogRead(PinThermo[i]);
  }

  while (true)
  {

    common = alpha * analogRead(PinCommon) + (1 - alpha) * common;

    for (int i = 0; i < NThermo; i++)
    {
      Temperature[i] = alpha * analogRead(PinThermo[i]) + (1 - alpha) * Temperature[i];
    }
    delay(1);
  }
}
