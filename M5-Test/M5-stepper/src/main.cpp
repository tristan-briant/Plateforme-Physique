#include <m5stack.h>

const bool TWO_COILS = 1;
const float thresholdHIGH = 50;
const float thresholdLOW = 60;

const int measurePin[] = {12, 13, 34, 15};
const int enablePin = 26;

bool Measure;

bool PosEdgeX, PosEdgeY;
bool first;
unsigned long TIMEOUT = 100000;

int stepPerRow = 400;
float Acc = 2e-4;
float SpeedM, Speed, SpeedTarget; // step per sec

unsigned long temp = 5000;
int motorPin[] = {16, 5, 17, 2};

float PeriodX, PeriodY;
float x, y;

float GetSpeed(unsigned long MeasureTime = 0);

void TaskCycle(void *)
{
  while (true)
  {
    SpeedTarget = 1000;
    delay(5000);
    /*SpeedTarget = -500;
    delay(5000);*/
  }
}

void TaskGUI(void *);
void TaskMeasure(void *);

void setup()
{
  M5.begin(true, false);

  for (int i = 0; i < 4; i++)
    pinMode(motorPin[i], OUTPUT);

  Serial.begin(2000000);

  xTaskCreatePinnedToCore(TaskGUI, "", 4000, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(TaskCycle, "", 4000, NULL, 1, NULL, 0);

  pinMode(25, OUTPUT);
  digitalWrite(25, 0);
  pinMode(26, OUTPUT);

  // pinMode(2, INPUT);
  //  analogSetWidth(9);
}

void TaskGUI(void *)
{

  while (true)
  {
    M5.update();

    if (M5.BtnA.wasPressed())
      SpeedTarget -= 100;
    if (M5.BtnB.wasPressed())
      SpeedTarget = 0;
    if (M5.BtnC.wasPressed())
      SpeedTarget += 100;

    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("%5.2f   %5.2f   %5.2f", Speed, SpeedTarget, SpeedM);

     Serial.println(SpeedM);
    //Serial.println(GetSpeed());
    delay(1);
    /*digitalWrite(enablePin, HIGH);
    for (int i = 0; i < 500; i++)
    {

      int I = analogRead(measurePin[0]); // - analogRead(measurePin[1]);
      // int Q = analogRead(measurePin[2]);// - analogRead(measurePin[3]);
      // Serial.printf("%5d  %5d\n", I, Q);
      Serial.printf("%5d  \n", I);
    }
    delay(500);*/
  }
}

void loop()
{
  static int count;
  count = (count + 1) % 100; // if count = 0 --> Measure

  // Time management
  static long timeOld = micros();
  long timeNew, deltaTime;
  timeNew = micros();
  deltaTime = timeNew - timeOld;
  timeOld = timeNew;
  /////////////////////////////

  if (count == 0 || Speed == 0)
  {
    delayMicroseconds(temp/2);
    SpeedM = GetSpeed();
  }

  if (Speed < SpeedTarget)
    Speed = min(SpeedTarget, Speed + Acc * deltaTime);
  if (Speed > SpeedTarget)
    Speed = max(SpeedTarget, Speed - Acc * deltaTime);

  if (Speed != 0)
  {
    digitalWrite(enablePin, HIGH);

    temp = min(1e6 / abs(Speed), 100000);

    int k = Speed > 0 ? 0 : 3;

    for (int i = 0; i < 4; i++)
    {
      digitalWrite(motorPin[k], HIGH);
      if (TWO_COILS)
        digitalWrite(motorPin[(k + 1) % 4], HIGH);

      delayMicroseconds(temp);

      digitalWrite(motorPin[k], LOW);
      if (TWO_COILS)
        digitalWrite(motorPin[(k + 1) % 4], LOW);

      k += Speed > 0 ? +1 : -1;
    }
  }
}

float GetSpeed(unsigned long MeasureTime)
{
  static float prevSpeed = 0;

  MeasureTime = min(MeasureTime, TIMEOUT);
  digitalWrite(enablePin, HIGH);

  float speed = 0;
  const float threshold = 100;
  int I, Q;
  bool StateI, StateQ;
  unsigned long timeEdgeI_P = 0, timeEdgeI_N, timeEdgeQ_P, timeEdgeQ_N;

  for (int i = 0; i < 4; i++)
    digitalWrite(motorPin[i], LOW);

  I = analogRead(measurePin[0]) - analogRead(measurePin[1]);
  Q = analogRead(measurePin[2]) - analogRead(measurePin[3]);

  if (MeasureTime == 0 && I * I + Q * Q < threshold * threshold) // not turning
    return 0;

  StateI = I > 0;
  StateQ = Q > 0;

  unsigned long timeStart = micros();

  while (micros() - timeStart < (MeasureTime != 0 ? MeasureTime : TIMEOUT))
  {
    I = analogRead(measurePin[0]); // - analogRead(measurePin[1]);
    // Q = analogRead(measurePin[2]) - analogRead(measurePin[3]);

    if (I < threshold / 2)
      StateI = LOW;

    if (StateI == LOW && I > threshold) // Detecte Edge on I
    {
      StateI = HIGH;
      if (timeEdgeI_P == 0)
        timeEdgeI_P = micros();
      else
      {
        speed = 1e6 / ((float)(micros() - timeEdgeI_P));

        if (speed > 100)
        {
          speed = prevSpeed > 0 ? speed : -speed;
        }
        else
        {
          Q = analogRead(measurePin[2]); // - analogRead(measurePin[3]);
          if (Q < threshold / 2)
            speed = -speed;
        }
        break;
      }
    }
  }

  if (MeasureTime > 0)
    while (micros() - timeStart < MeasureTime)
      ;


  prevSpeed = speed;
  return speed; // not edge detected before Timout
}
