#include <Arduino.h>

const bool TWO_COILS = 1;
const float threshold=30;

int temp = 2500;
int motorPin[] = {18, 19, 23, 22};

float timing;
float x, y;

float speed;

void TaskGUI(void *);

void setup()
{
  for (int i = 0; i < 4; i++)
    pinMode(motorPin[i], OUTPUT);

  Serial.begin(2000000);

  xTaskCreatePinnedToCore(TaskGUI, "toto", 4000, NULL, 1, NULL, 0);

  pinMode(25, OUTPUT);
  digitalWrite(25, 0);

  pinMode(2, INPUT);
  // analogSetWidth(9);
}

void TaskGUI(void *)
{

  while (true)
  {
    // Serial.println(timing);
    // if (x > 100)
    // for (int i = 0; i < 500; i++)
    {

      Serial.printf("%5f %5f  %5f\n", 10000000.0 / timing, x, y);
      // delay(1);
      delayMicroseconds(100);
    }

    delay(10);
  }
}

bool PosEdgeX, PosEdgePY;
bool first;
int TIMEOUT = 100000;

void loop()
{
  const float alpha = 0.5;
  float xnew, ynew;
  static int told = 0;
  int tnew;

  // x = analogReadMilliVolts(35);
  xnew = alpha * xnew + (1 - alpha) * analogRead(35);
  ynew = alpha * ynew + (1 - alpha) * analogRead(36);

  tnew = micros();

  if (!PosEdgeX && xnew > threshold)
  {
    PosEdgeX = true;
    timing = tnew - told;
    told = tnew;
  }

  if (xnew < threshold-10)
    PosEdgeX = false;

  if (tnew - told > timing)
    timing = tnew - told;

  x = xnew;
  y = ynew;
  // x=digitalRead(2);
  // y=digitalRead(5);
  // timing = micros() - told;
  // Serial.println(timing);
  // Serial.printf("%5d  %5x\n", timing,x);
  /*or (int i = 0; i < 4; i++)
  {
    digitalWrite(motorPin[i], HIGH);
    if (TWO_COILS)
      digitalWrite(motorPin[(i + 1) % 4], HIGH);

    delayMicroseconds(temp);
    digitalWrite(motorPin[i], LOW);
    if (TWO_COILS)
      digitalWrite(motorPin[(i + 1) % 4], LOW);
  }*/
}