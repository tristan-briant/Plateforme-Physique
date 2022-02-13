#include <Arduino.h>

/*const int PinCHAdir = 2;
const int PinCHBdir = 17;
const int PinCHAen = 16;
const int PinCHBen = 5;*/

/*const int PinCHApow = 22;
const int PinCHBpow = 21;*/

extern int mode;

/////// Config 1 //////////
const int PinIn1 = 2;
const int PinIn2 = 17;
const int PinIn3 = 16;
const int PinIn4 = 5;

/////// Config 2 //////////
/*const int PinIn1 = 16;
const int PinIn2 = 17;
const int PinIn3 = 2;
const int PinIn4 = 5;
*/
int delaylength = 100;
extern int MicroStep;
extern bool isOn;
extern float power;
const int freqPWM = 20000;
const int MaxPWM = 2047;
const int NbitPWM = 11;

void motor_init()
{

  /*pinMode(PinCHAdir, OUTPUT);
  pinMode(PinCHBdir, OUTPUT);
  pinMode(PinCHAen, OUTPUT);
  pinMode(PinCHBen, OUTPUT);*/

  pinMode(PinIn1, OUTPUT);
  pinMode(PinIn2, OUTPUT);
  pinMode(PinIn3, OUTPUT);
  pinMode(PinIn4, OUTPUT);

  //pinMode(PinCHApow, OUTPUT); //brake (disable) CH A
  //pinMode(PinCHBpow, OUTPUT); //brake (disable) CH B

  ledcSetup(0, freqPWM, NbitPWM);
  ledcSetup(1, freqPWM, NbitPWM);

  //ledcAttachPin(PinCHApow, 0);
  //ledcAttachPin(PinCHBpow, 1);

  ledcAttachPin(PinIn1, 0);
  ledcAttachPin(PinIn3, 1);

  //pinMode(PinCHApow, OUTPUT); //brake (disable) CH A
  //pinMode(PinCHBpow, OUTPUT); //brake (disable) CH B

  //digitalWrite(PinCHApow, HIGH);
  //digitalWrite(PinCHBpow, HIGH);
}

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

float triangle(float x)
{
  float y = x - floor(x);

  if (y < 0.5)
    return 4 * y - 1;
  else
    return 3 - 4 * y;
}

void ReleaseMotor()
{
  digitalWrite(PinIn2, LOW);
  digitalWrite(PinIn4, LOW);
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

  //cosphi = triangle(phi / 2 / PI + 0.25);
  //sinphi = triangle(phi / 2 / PI);

  if (cosphi > 0)
  {
    digitalWrite(PinIn2, HIGH);
    ledcWrite(1, floor(MaxPWM * f(1 - power * cosphi)));
  }
  else
  {
    digitalWrite(PinIn2, LOW);
    ledcWrite(1, floor(MaxPWM * power * f(-cosphi)));
  }

  if (sinphi > 0)
  {
    digitalWrite(PinIn4, HIGH);
    ledcWrite(0, floor(MaxPWM * f(1 - power * sinphi)));
  }
  else
  {
    digitalWrite(PinIn4, LOW);
    ledcWrite(0, floor(MaxPWM * power * f(-sinphi)));
  }
}

//int step = 0;
/*
void oneStep2(bool forward)
{

  if (forward)
    step++;
  else
    step--;
  step = step % 8;
  if (step < 0)
    step = step + 8;

  switch (step)
  {
  case 0:
    digitalWrite(PinCHAen, LOW);   //ENABLE CH A
    digitalWrite(PinCHBen, HIGH);  //DISABLE CH B
    digitalWrite(PinCHAdir, HIGH); //Sets direction of CH A
    digitalWrite(PinCHApow, HIGH); //Moves CH A
    break;
  case 1:
    digitalWrite(PinCHAen, LOW);   //ENABLE CH A
    digitalWrite(PinCHBen, LOW);   //ENABLE CH B
    digitalWrite(PinCHAdir, HIGH); //Sets direction of CH A
    digitalWrite(PinCHBdir, LOW);  //Sets direction of CH B
    digitalWrite(PinCHBpow, HIGH); //Moves CH B
    digitalWrite(PinCHApow, HIGH); //Moves CH A
    break;

  case 2:
    digitalWrite(PinCHAen, HIGH);  //DISABLE CH A
    digitalWrite(PinCHBen, LOW);   //ENABLE CH B
    digitalWrite(PinCHBdir, LOW);  //Sets direction of CH B
    digitalWrite(PinCHBpow, HIGH); //Moves CH B
    break;

  case 3:
    digitalWrite(PinCHAen, LOW);   //ENABLE CH A
    digitalWrite(PinCHBen, LOW);   //ENABLE CH B
    digitalWrite(PinCHAdir, LOW);  //Sets direction of CH A
    digitalWrite(PinCHBdir, LOW);  //Sets direction of CH B
    digitalWrite(PinCHBpow, HIGH); //Moves CH B
    digitalWrite(PinCHApow, HIGH); //Moves CH A
    break;

  case 4:
    digitalWrite(PinCHAen, LOW);   //ENABLE CH A
    digitalWrite(PinCHBen, HIGH);  //DISABLE CH B
    digitalWrite(PinCHAdir, LOW);  //Sets direction of CH A
    digitalWrite(PinCHApow, HIGH); //Moves CH A
    break;

  case 5:
    digitalWrite(PinCHAen, LOW);   //ENABLE CH A
    digitalWrite(PinCHBen, LOW);   //ENABLE CH B
    digitalWrite(PinCHAdir, LOW);  //Sets direction of CH A
    digitalWrite(PinCHBdir, HIGH); //Sets direction of CH B
    digitalWrite(PinCHBpow, HIGH); //Moves CH B
    digitalWrite(PinCHApow, HIGH); //Moves CH A
    break;

  case 6:
    digitalWrite(PinCHAen, HIGH);  //DISABLE CH A
    digitalWrite(PinCHBen, LOW);   //ENABLE CH B
    digitalWrite(PinCHBdir, HIGH); //Sets direction of CH B
    digitalWrite(PinCHBpow, HIGH); //Moves CH B
    break;

  case 7:
    digitalWrite(PinCHAen, LOW);   //ENABLE CH A
    digitalWrite(PinCHBen, LOW);   //ENABLE CH B
    digitalWrite(PinCHAdir, HIGH); //Sets direction of CH A
    digitalWrite(PinCHBdir, HIGH); //Sets direction of CH B
    digitalWrite(PinCHBpow, HIGH); //Moves CH B
    digitalWrite(PinCHApow, HIGH); //Moves CH A
    break;
  }

  delayMicroseconds(delaylength);
  //analogWrite(11, 100);
  //analogWrite(3, 100);
  //digitalWrite(9, HIGH); //DISABLE CH A
  //digitalWrite(8, HIGH); //DISABLE CH B
}

void oneStep3(bool forward)
{

  if (forward)
    step++;
  else
    step--;
  step = step % 8;
  if (step < 0)
    step = step + 8;

  switch (step)
  {
  case 0:
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, HIGH);
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, LOW);
    break;
  case 1:
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, HIGH);
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, HIGH);
    break;

  case 2:
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, LOW);
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, HIGH);
    break;

  case 3:
    digitalWrite(PinIn1, HIGH);
    digitalWrite(PinIn2, LOW);
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, HIGH);
    break;

  case 4:
    digitalWrite(PinIn1, HIGH);
    digitalWrite(PinIn2, LOW);
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, LOW);
    break;

  case 5:
    digitalWrite(PinIn1, HIGH);
    digitalWrite(PinIn2, LOW);
    digitalWrite(PinIn3, HIGH);
    digitalWrite(PinIn4, LOW);
    break;

  case 6:
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, LOW);
    digitalWrite(PinIn3, HIGH);
    digitalWrite(PinIn4, LOW);
    break;

  case 7:
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, HIGH);
    digitalWrite(PinIn3, HIGH);
    digitalWrite(PinIn4, LOW);
    break;
  }

  delayMicroseconds(delaylength);
  //analogWrite(11, 100);
  //analogWrite(3, 100);
  //digitalWrite(9, HIGH); //DISABLE CH A
  //digitalWrite(8, HIGH); //DISABLE CH B
}

void oneStep4(bool forward)
{

  const int MicroStep = 8;

  if (forward)
    step++;
  else
    step--;

  step = step % (4 * MicroStep);
  if (step < 0)
    step = step + 4 * MicroStep;

  int mstep = step % MicroStep;
  int pwm = mstep * 256 / MicroStep;

  switch (step / MicroStep)
  {
  case 0:
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, HIGH);
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, HIGH);
    ledcWrite(0, 255 - pwm);
    ledcWrite(1, pwm);
    break;

  case 1:
    digitalWrite(PinIn1, HIGH);
    digitalWrite(PinIn2, LOW);
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, HIGH);
    ledcWrite(0, pwm);
    ledcWrite(1, 255 - pwm);
    break;

  case 2:
    digitalWrite(PinIn1, HIGH);
    digitalWrite(PinIn2, LOW);
    digitalWrite(PinIn3, HIGH);
    digitalWrite(PinIn4, LOW);
    ledcWrite(0, 255 - pwm);
    ledcWrite(1, pwm);
    break;

  case 3:
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, HIGH);
    digitalWrite(PinIn3, HIGH);
    digitalWrite(PinIn4, LOW);
    ledcWrite(0, pwm);
    ledcWrite(1, 255 - pwm);
    break;
  }

  delayMicroseconds(delaylength);
  //analogWrite(11, 100);
  //analogWrite(3, 100);
  //digitalWrite(9, HIGH); //DISABLE CH A
  //digitalWrite(8, HIGH); //DISABLE CH B
}

void oneStep5(bool forward)
{
    static int step =0;
  if (forward)
    step++;
  else
    step--;

  step = step % MicroStep;
  if (step < 0)
    step = step + MicroStep;

  float phi = 2 * PI * step / (float)MicroStep;

  float cosphi = cos(phi);
  float sinphi = sin(phi);

  if (cosphi > 0)
  {
    digitalWrite(PinIn1, LOW);
    digitalWrite(PinIn2, HIGH);
    ledcWrite(0, int(255 * cosphi));
  }
  else
  {
    digitalWrite(PinIn1, HIGH);
    digitalWrite(PinIn2, LOW);
    ledcWrite(0, int(255 * -cosphi));
  }

  if (sinphi > 0)
  {
    digitalWrite(PinIn3, LOW);
    digitalWrite(PinIn4, HIGH);
    ledcWrite(1, int(255 * sinphi));
  }
  else
  {
    digitalWrite(PinIn3, HIGH);
    digitalWrite(PinIn4, LOW);
    ledcWrite(1, int(255 * -sinphi));
  }

  delayMicroseconds(delaylength);
}
*/
