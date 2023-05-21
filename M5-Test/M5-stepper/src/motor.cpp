#include <Arduino.h>

// extern int mode;

/////// Pin Config //////////
const int PinIn1 = 2;
const int PinIn2 = 5;
const int PinIn3 = 16;
const int PinIn4 = 17;

float puissance = 0.5;

bool isOn = true;
float power = 1;
const int freqPWM = 15000;
const int MaxPWM = 4095;//1023;
const int NbitPWM = 12;

void motor_init()
{
  pinMode(PinIn1, OUTPUT);
  pinMode(PinIn2, OUTPUT);
  pinMode(PinIn3, OUTPUT);
  pinMode(PinIn4, OUTPUT);

  ledcSetup(0, freqPWM, NbitPWM);
  ledcSetup(1, freqPWM, NbitPWM);
  ledcSetup(2, freqPWM, NbitPWM); // test
  ledcSetup(3, freqPWM, NbitPWM); // test

  ledcAttachPin(PinIn1, 0);
  ledcAttachPin(PinIn3, 1);
  ledcAttachPin(PinIn2, 2); // test
  ledcAttachPin(PinIn4, 3); // test
}

const int mode = 3;
float f(float x)
{
  if (mode == 0)
    return x;
  if (mode == 1)
    return pow(x, 2);
  if (mode == 2)
    return sqrt(x);
  if (mode == 3)
    return pow(x, 0.75);
  if (mode == 4) 
    return pow(x, puissance);
  return x;
}

void ReleaseMotor()
{
  digitalWrite(PinIn4, LOW);
  digitalWrite(PinIn2, LOW);
  ledcWrite(1, 0);
  ledcWrite(0, 0);
}

extern float coscos, sinsin;

void oneStep(float forward)
{
  static float step = 0;
  float X1, X2; // Motor quadrature

  step += forward;

  // float phi = 2 * PI * step / (float)MicroStep;
  float phi = 4 * step; // / (float)MicroStep;
  float cosphi, sinphi;

  // coscos = cosphi = cos(phi);
  // sinsin = sinphi = sin(phi);

  float a = fmod(phi, 4);
  if (a < 0)
    a += 4;

  if (a < 1)
  {
    X1 = 2 * a - 1;
    X2 = 1;
  }
  else if (a < 2)
  {
    X1 = 1;
    X2 = 1 - 2 * (a - 1);
  }
  else if (a < 3)
  {
    X1 = 1 - 2 * (a - 2);
    X2 = -1;
  }
  else
  {
    X1 = -1;
    X2 = 2 * (a - 3) - 1;
  }

  //X1=cos(phi*PI/2);
  //X2 = sin(phi*PI/2);

  coscos = X1;
  sinsin = X2;

  if (X1 >= 0)
  {
    // digitalWrite(PinIn4, HIGH);
    ledcWrite(3, floor(MaxPWM * power * f(X1)));
    // ledcWrite(1, floor(MaxPWM * f(1 - power * cosphi)));
    ledcWrite(1, 0);
  }
  else
  {
    // digitalWrite(PinIn4, LOW);
    ledcWrite(3, 0);
    ledcWrite(1, floor(MaxPWM * power * f(-X1)));
  }

  if (X2 >= 0)
  {
    // digitalWrite(PinIn2, HIGH);
    ledcWrite(2,  floor(MaxPWM * power * f(X2)));
    //ledcWrite(0, floor(MaxPWM * f(1 - power * X2)));
    ledcWrite(0, 0);
  }
  else
  {
    // digitalWrite(PinIn2, LOW);
    ledcWrite(2, 0);
    ledcWrite(0, floor(MaxPWM * power * f(-X2)));
  }
}
