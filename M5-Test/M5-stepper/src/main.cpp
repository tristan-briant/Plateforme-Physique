#include <Arduino.h>

const bool TWO_COILS = 1;

int temp = 2500;
int motorPin[] = {18, 19, 23, 22};

void setup() {
  for (int i = 0; i < 4; i++)
    pinMode(motorPin[i], OUTPUT);
}

void loop() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(motorPin[i], HIGH);
    if(TWO_COILS) digitalWrite(motorPin[(i+1)%4], HIGH);
    
    delayMicroseconds(temp);
    digitalWrite(motorPin[i], LOW);
    if(TWO_COILS) digitalWrite(motorPin[(i+1)%4], LOW);
  }
}