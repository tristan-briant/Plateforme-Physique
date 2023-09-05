#include <m5stack.h>

int TEMP = 2000;

int PinMotor1[] = {16, 17, 2, 5};
int PinMotor2[] = {21, 22, 23, 19};

void setup()
{
  M5.begin(true, false, false, false);

  for (int i = 0; i < 4; i++)
  {
    pinMode(PinMotor1[i], OUTPUT);
    // pinMode(PinMotor2[i], OUTPUT);
  }
}

void loop()
{

  int count = 0;
  int N = 800;

  bool motorOn = true;
  unsigned int c1, c2 = 0;
  int *p1 = PinMotor1;
  int *p2 = PinMotor2;

  int direction1 = 1;
  int direction2 = -1;

  int tempMIN=2000;
  int tempMAX=4000;


  while (true)
  { // never return

    if (count < 100)
      TEMP = map(count, 0, 100, tempMAX, tempMIN);
    else if (count > N - 100)
    {
      TEMP = map(count, N - 100, N, tempMIN, tempMAX);
    }
    else
      TEMP = tempMIN;

    if (motorOn)
    {
      digitalWrite(p1[c1], HIGH);
      digitalWrite(p1[(c1 + 1) % 4], HIGH);
    }
    if (motorOn)
    {
      digitalWrite(p2[c2], HIGH);
      digitalWrite(p2[(c2 + 1) % 4], HIGH);
    }
    delayMicroseconds(TEMP);

    if (direction1 == 1)
      digitalWrite(p1[c1], LOW);
    if (direction1 == -1)
      digitalWrite(p1[(c1 + 1) % 4], LOW);

    if (direction2 == 1)
      digitalWrite(p2[c2], LOW);
    if (direction2 == -1)
      digitalWrite(p2[(c2 + 1) % 4], LOW);

    c1 = (c1 + direction1) % 4;
    c2 = (c2 + direction2) % 4;

    count++;
    if (count > N)
    {
      direction1 = -direction1;
      count = 0;
    }
  }
}
