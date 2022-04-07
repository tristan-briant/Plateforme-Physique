#include <M5core2.h>

/////// Pin Config //////////
int PinMotor[] = {13, 14, 32, 33};
int PinMotor2[] = {27, 19, 2, 0};

const int N = 100;
const int TEMP = 5000;

void step(int motor, int dir);

void setup()
{
M5.begin();
M5.Lcd.print("coucou");

  for (int i = 0; i < 4; i++)
  {
    pinMode(PinMotor[i], OUTPUT);
    pinMode(PinMotor2[i], OUTPUT);
  }
}

void loop()
{
  //M5.Lcd.print("coucou");

  for (int i = 0; i < N; i++)
  {
    step(1,1);
    //step(2,1);
  }

  for (int i = 0; i < N; i++)
  {
    step(1,-1);
    //step(2,-1);
  }
}

void step(int motor, int dir)
{
  int *p;
  if (motor == 1)
    p = PinMotor;
  else
    p = PinMotor2;

  if (dir > 0)
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(p[i], HIGH);
      digitalWrite(p[(i + 1) % 4], HIGH);
      delayMicroseconds(TEMP);
      digitalWrite(p[i], LOW);
      digitalWrite(p[(i + 1) % 4], LOW);
    }
  else
    for (int i = 3; i >= 0; i--)
    {
      digitalWrite(p[i], HIGH);
      digitalWrite(p[(i + 1) % 4], HIGH);
      delayMicroseconds(TEMP);
      digitalWrite(p[i], LOW);
      digitalWrite(p[(i + 1) % 4], LOW);
    }
}