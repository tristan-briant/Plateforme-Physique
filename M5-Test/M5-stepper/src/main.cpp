#include <m5stack.h>

void oneStep(int forward);
void motor_init();
void ReleaseMotor();
float phase = 0;

#define ANALOG_READ 1

const int OFLedPin = 12; // Optical fork LED
const int OFPhDPin = 34; // Photodiode

const float TIME_TO_ROW = 1e6 / 20; // Speed in row per second

const bool TWO_COILS = 1;
const float thresholdHIGH = 50;
const float thresholdLOW = 60;

// const int measurePin[] = {12, 13, 34, 15};
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

  motor_init();

  xTaskCreatePinnedToCore(TaskGUI, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskMeasure, "", 4000, NULL, 1, NULL, 1);

  pinMode(25, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(25, 0);
  pinMode(OFLedPin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  //Speed = 100;
}

bool phd;

void TaskGUI(void *)
{

  while (true)
  {
    M5.update();

    if (M5.BtnA.wasPressed())
      SpeedTarget -= 1;
    if (M5.BtnB.wasPressed())
      SpeedTarget = 0;
    if (M5.BtnC.wasPressed())
      SpeedTarget += 1;

    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("%5.2f   %5.2f   %5.2f     %5.2f", Speed, SpeedTarget, SpeedM,phase);

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("%d", phd);

    Serial.println(SpeedM);
    delay(10);
  }
}

void loop()
{
  
  int s;
  while (true)
  {

    // Time management
    static long timeOld = micros();
    long timeNew, deltaTime;
    timeNew = micros();
    deltaTime = timeNew - timeOld;
    timeOld = timeNew;
    /////////////////////////////

    if (Speed < SpeedTarget)
      Speed = min(SpeedTarget, Speed + Acc * deltaTime);
    if (Speed > SpeedTarget)
      Speed = max(SpeedTarget, Speed - Acc * deltaTime);

    phase += Speed * deltaTime/1e3;

    if (s < phase)
    {
      oneStep(1);
      s++;
    }
    else if (s > phase + 1)
    {
      oneStep(-1);
      s--;
    }
    else
      oneStep(0);

    delayMicroseconds(5);
  }
}

void TaskMeasure(void *param)
{

  pinMode(OFPhDPin, INPUT);
  digitalWrite(OFLedPin, HIGH);

  unsigned long timeStart, timeEnd, timeEdge;
  bool StateHigh = digitalRead(OFPhDPin);
  long interval;

  while (true)
  {
    bool x = digitalRead(OFPhDPin);
    phd = x;

    unsigned long t = micros();

    if (x && !StateHigh)
    {
      StateHigh = true;
      interval = t - timeEdge;

      SpeedM = TIME_TO_ROW / interval;
      timeEdge = t;
    }

    if (!x)
      StateHigh = false;

    if (t - timeEdge > 2 * interval)
    {
      interval = 2 * interval;
      SpeedM = TIME_TO_ROW / interval;
    }

    delay(1);
  }
}