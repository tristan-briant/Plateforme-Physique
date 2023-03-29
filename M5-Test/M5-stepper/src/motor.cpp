#include <Arduino.h>

//extern int mode;

/////// Pin Config //////////
const int PinIn1 = 2;
const int PinIn2 = 5;
const int PinIn3 = 16;
const int PinIn4 = 17;

int delaylength = 100;
int MicroStep = 64;
bool isOn = true;
float power=1;
const int freqPWM = 20000;
const int MaxPWM = 2047;
const int NbitPWM = 11;

void motor_init()
{
  pinMode(PinIn1, OUTPUT);
  pinMode(PinIn2, OUTPUT);
  pinMode(PinIn3, OUTPUT);
  pinMode(PinIn4, OUTPUT);

  ledcSetup(0, freqPWM, NbitPWM);
  ledcSetup(1, freqPWM, NbitPWM);

  ledcAttachPin(PinIn1, 0);
  ledcAttachPin(PinIn3, 1);
}

const int mode=0;
float f(float x)
{
  if (mode == 0)
    return x;
  if (mode == 1)
    return pow(x, 2);
  if (mode == 2)
    return sqrt(x);
  if (mode == 3)
    return pow(x, 1.5);
  if (mode == 4)
    return pow(x, 0.8);
  return x;
}

void ReleaseMotor()
{
  digitalWrite(PinIn4, LOW);
  digitalWrite(PinIn2, LOW);
  ledcWrite(1, 0);
  ledcWrite(0, 0);
}

void oneStep(int forward)
{
  static int step = 0;

  step += forward;

  float phi = 2 * PI * step / (float)MicroStep;

  float cosphi, sinphi;

  cosphi = cos(phi);
  sinphi = sin(phi);

  if (cosphi > 0)
  {
    digitalWrite(PinIn4, HIGH);
    ledcWrite(1, floor(MaxPWM * f(1 - power * cosphi)));
  }
  else
  {
    digitalWrite(PinIn4, LOW);
    ledcWrite(1, floor(MaxPWM * power * f(-cosphi)));
  }

  if (sinphi > 0)
  {
    digitalWrite(PinIn2, HIGH);
    ledcWrite(0, floor(MaxPWM * f(1 - power * sinphi)));
  }
  else
  {
    digitalWrite(PinIn2, LOW);
    ledcWrite(0, floor(MaxPWM * power * f(-sinphi)));
  }
}
