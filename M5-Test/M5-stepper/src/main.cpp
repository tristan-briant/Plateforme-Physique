#include <m5stack.h>

void oneStep(float forward);
void motor_init();
void ReleaseMotor();
float phase = 0;
extern float power;
extern float puissance;

#define ANALOG_READ 1

const int OFLedPin = 12; // Optical fork LED
const int OFPhDPin = 34; // Photodiode

const bool TWO_COILS = 1;
const float thresholdHIGH = 50;
const float thresholdLOW = 60;

// const int measurePin[] = {12, 13, 34, 15};
const int enablePin = 26;

bool Measure;

bool PosEdgeX, PosEdgeY;
bool first;
unsigned long TIMEOUT = 100000;

// Speed in turn per second
int stepPerRow = 100;
float Acc = 0.1;                  // turn per sec^2
float SpeedM, Speed, SpeedTarget; // row per sec

// const float SensorPeriodToSpeed = 1.0e-2;  //100 ticks
const float TIME_TO_ROW = 1.0e-2; // Speed in row per second speed(row per s) = TIME_TO_ROW / interval(second)

unsigned long temp = 5000;
int motorPin[] = {16, 5, 17, 2};

float PeriodX, PeriodY;
float x, y;

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
  // xTaskCreatePinnedToCore(TaskMeasure, "", 4000, NULL, 1, NULL, 1);

  pinMode(25, OUTPUT);
  digitalWrite(25, 0);

  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  pinMode(OFPhDPin, INPUT);
  pinMode(OFLedPin, OUTPUT);
  digitalWrite(OFLedPin, HIGH);
}

int phd;

float coscos, sinsin;
void TaskGUI(void *)
{

  while (true)
  {
    M5.update();

    if (M5.BtnA.wasPressed()||M5.BtnA.pressedFor(200))
      SpeedTarget -= 0.01; 
      //puissance-=0.01;    
    if (M5.BtnB.wasPressed())
      SpeedTarget = Speed = phase = 0;
    if (M5.BtnC.wasPressed()||M5.BtnC.pressedFor(200))
      SpeedTarget += 0.01;
      //puissance+=0.01; 

    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("%5.3f   %5.3f   %5.3f     %5.3f", Speed, SpeedTarget, SpeedM, puissance);

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("%10d", phd);

    float r = 50;
    float x = r * coscos, y = r * sinsin;

    M5.Lcd.fillCircle(160, 120, r, BLACK);
    M5.Lcd.drawLine(160, 120, 160 + x, 120 + y, RED);

    Serial.println(SpeedM);
    //SpeedTarget=0.01;
    delay(10);
  }
}

void loop()
{
  static unsigned long timeEdge;
  static bool StateHigh = digitalRead(OFPhDPin);
  static long interval;

  static float s = 0;

  // Time management
  static long timeOld = micros();
  long timeNew;
  float deltaTime;
  timeNew = micros();
  deltaTime = (timeNew - timeOld) * 1e-6;
  timeOld = timeNew;
  /////////////////////////////

  if (Speed < SpeedTarget)
    Speed = min(SpeedTarget, Speed + Acc * deltaTime);
  if (Speed > SpeedTarget)
    Speed = max(SpeedTarget, Speed - Acc * deltaTime);

  /*if (SpeedM < fabs(Speed) - 0.2 * fabs(SpeedTarget))
    if (SpeedTarget > 0)
      Speed = SpeedM;
    else
      Speed = -SpeedM;
*/
  /*if (abs(Speed - SpeedTarget) > 0.01)
    power = 1;
  else
  {
    if (abs(SpeedTarget) < 1)
      power = 0.5;
    else if (abs(SpeedTarget) < 2)
      power = 0.75;
    else
      power = 1;
  }*/
  if (SpeedTarget == 0)
    power = 0;
  else
    power = 0.95;

  oneStep((Speed * deltaTime) * stepPerRow);

  //////////////////////////////////// Speed measurement//////////////////////////////////////

  bool x = digitalRead(OFPhDPin);
  phd = interval;
  unsigned long t = micros();

  if (!x)
    StateHigh = false;

  if (x && !StateHigh)
  {
    StateHigh = true;
    interval = t - timeEdge;
    const float alpha = 0.05;
    SpeedM = alpha * SpeedM + (1 - alpha) * TIME_TO_ROW / interval * 1e6;
    timeEdge = t;
  }

  if (t - timeEdge > interval + 10000)
  {
    interval = interval + 10000;
    SpeedM = TIME_TO_ROW / interval * 1e6;
  }

  delayMicroseconds(1);
}

/*
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

      SpeedM = TIME_TO_ROW / interval * 1e6;
      timeEdge = t;
    }

    if (!x)
      StateHigh = false;

    if (t - timeEdge > interval + 1000)
    {
      interval = interval + 1000;
      SpeedM = TIME_TO_ROW / interval * 1e6;
    }

    delay(1);
  }
}
*/