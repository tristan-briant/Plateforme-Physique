#include <M5Stack.h>

int temp = 3000;
int motorPin[] = {16, 17, 2, 5};
int MotorSelectPin = 26;
int x_set, x;

int motorNumber = 0;

TaskHandle_t Task1, Task2;

void steps(int motor, bool forward);
void UserITask(void *param);
void MotorTask(void *param);

void setup()
{
  M5.begin();
  M5.Lcd.setTextSize(2);

  for (int i = 0; i < 4; i++)
    pinMode(motorPin[i], OUTPUT);

  pinMode(MotorSelectPin, OUTPUT);

  xTaskCreatePinnedToCore(UserITask, "", 4000, NULL, 1, &Task1, 0); // User Interface on core 0
  xTaskCreatePinnedToCore(MotorTask, "", 4000, NULL, 1, &Task2, 1); // Motor Driver on core 1
}

void loop()
{ // Not used
}

void UserITask(void *param)
{ /// User interface
  while (true)
  {              // never return
    M5.update(); // read button status

    if (M5.BtnA.isPressed())
      x_set -= 10;

    if (M5.BtnC.isPressed())
      x_set += 10;

    if (M5.BtnB.wasReleased())
      motorNumber = 1 - motorNumber;

    M5.Lcd.setCursor(50, 120);
    M5.Lcd.printf("x = %4d :: %4d", x, x_set);

    M5.Lcd.setCursor(50, 200);
    M5.Lcd.printf("motor = %d ", motorNumber);

    delay(20);
  }
}

void MotorTask(void *param)
{ /// Motor driver
  while (true)
  { // never return

    if (x < x_set)
    { // feedback loop on motor position
      steps(motorNumber, true);
      x++;
    }

    if (x > x_set)
    {
      steps(motorNumber, false);
      x--;
    }
  }
}

void steps(int motor, bool forward)
{ // make 4 steps in forward or backward direction
  if (motor == 0)
    digitalWrite(MotorSelectPin, HIGH);
  else
    digitalWrite(MotorSelectPin, LOW);

  if (forward == true)
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(motorPin[i], HIGH);
      delayMicroseconds(temp);
      digitalWrite(motorPin[i], LOW);
    }
  else
    for (int i = 3; i >= 0; i--)
    {
      digitalWrite(motorPin[i], HIGH);
      delayMicroseconds(temp);
      digitalWrite(motorPin[i], LOW);
    }
}