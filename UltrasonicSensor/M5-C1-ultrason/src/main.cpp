#include <M5stack.h>

float I, Q;

long t_loop_us;

float turn = 0;
float turn_frac, turnTotal, turn_frac0 = 0;
float turnTotal_avg = 0;
const float epsilon = 0.02;

TFT_eSprite img(&M5.Lcd);

void TaskGUI(void *param);

void setup()
{
  M5.begin();
  Serial.begin(115200);
  Serial2.begin(2000000, SERIAL_8N1, 22, 21);

  xTaskCreatePinnedToCore(TaskGUI, NULL, 4000, NULL, 0, NULL, 0);

  img.setColorDepth(8);
  img.createSprite(320, 40);

  /*while (true)
   {
     if (Serial2.available() && Serial2.read() == 255)
     {
       char x[5];
       Serial2.readBytes(x, 4);
       //I = (x[0] * 256 + x[1] - 32768) / 32767.0;
       //Q = (x[2] * 256 + x[3] - 32768) / 32767.0;

       // I = (i - 128) / 128.0;
       // Q = (q - 128) / 128.0;

       // I = (i - 32768) / 32767;
       // Q = (q - 32768) / 32767;
       //  angle = atan2f(Q, I);

       break;
     }
   }*/
}

void TaskGUI(void *param)
{
  float I_old, Q_old;
  static float angle_old = 0;
  const int x0 = 160, y0 = 130;
  const int r = 100;
  int r1, r2;
  float cosin, sinus;

  r1 = 80, r2 = 85;
  const int N_TICKS = 10;
  for (int i = 0; i < N_TICKS; i++)
  {
    cosin = cos(2 * PI * i / N_TICKS); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(2 * PI * i / N_TICKS); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, WHITE);
  }

  while (true)
  {
    M5.update();
    if (M5.BtnA.isPressed())
    {
      turn = 0;
      turn_frac0 = turn_frac;
      turnTotal_avg = turn_frac;
    }

    float angle = (turnTotal_avg - turn_frac0) * 2 * PI * 4.5;

    r1 = 25, r2 = 45;
    cosin = cos(angle_old); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle_old); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, BLACK);
    cosin = cos(angle); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle);
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, YELLOW);

    r1 = 50, r2 = 75;
    cosin = cos(angle_old / 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle_old / 10); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, BLACK);
    cosin = cos(angle / 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle / 10);
    M5.Lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, GREEN);

    r1 = 10, r2 = 20;
    cosin = cos(angle_old * 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle_old * 10); // epsilon * Q + (1 - epsilon) * Q_old;
    M5.lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, BLACK);
    cosin = cos(angle * 10); // epsilon * I + (1 - epsilon) * I_old;
    sinus = sin(angle * 10);
    M5.lcd.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, CYAN);

    img.fillSprite(BLACK);
    char str[80];
    // sprintf(str, "%10.2f", (turnTotal_avg - turn_frac0) * 4.25);
    img.setTextColor(WHITE);
    img.setTextDatum(BL_DATUM);
    img.setFreeFont(&FreeMono18pt7b);

    /* int x = abs((turnTotal_avg - turn_frac0) * 4.25 * 1000);
     int dec = (turnTotal_avg - turn_frac0) * 4.25;
     int frac1 = */

    float distance = -(turnTotal_avg - turn_frac0) * 4.25;

    sprintf(str, "%8.3f mm", distance);
    img.drawString(str, 50, 30);

    /*img.setTextColor(YELLOW);
    sprintf(str, "%02d", abs((int)(distance * 100) % 100));
    img.drawString(str, 50 + 6 * 18, 30);

    sprintf(str, "%1d", abs((int)(1000 * distance) % 10));
    img.setTextDatum(BR_DATUM);
    img.setFreeFont(&FreeMono12pt7b);
    img.drawString(str, 50 + 9 * 18, 27);*/

    // M5.Lcd.setTextSize(3);
    //  M5.Lcd.printf("%7.3f %7.3f", (turnTotal_avg - turn_frac0), (turnTotal_avg - turn_frac0) * 4.25);

    img.pushSprite(0, 0);

    angle_old = angle;
    // I_old = I_New;
    // Q_old = Q_New;

    M5.Lcd.setCursor(0, 220);
    M5.Lcd.printf("%4d", t_loop_us);
    delay(25);
  }
}

void loop()
{

  static long t_request;
  static bool data_requested = false;
  static int count = 0;
  static float I_old, Q_old;
  char x[5];
  // Serial2.flush();

  if (!data_requested || (micros() > t_request + 500))
  {
    data_requested = true;
    t_request = micros();
    Serial2.write(0);
  }

  //Serial2.write(0);

  if (Serial2.available())
  {
    static long t_loop_old;
    t_loop_us = micros() - t_loop_old;
    t_loop_old = micros();

    count++;
    Serial2.readBytes(x, 4);
    I = (x[0] * 256 + x[1] - 32768) / 32767.0;
    Q = (x[2] * 256 + x[3] - 32768) / 32767.0;

    if (Q >= 0 && Q_old < 0 && I < 0)
      turn--;
    if (Q < 0 && Q_old >= 0 && I < 0)
      turn++;

    turn_frac = atan2f(Q, I) / PI / 2.0;
    turnTotal = (turn + turn_frac);

    turnTotal_avg = epsilon * turnTotal + (1 - epsilon) * turnTotal_avg;

    data_requested = false;
    Q_old = Q;
    I_old = I;
  }

}
