#include <Arduino.h>
#include <M5Stack.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <SD.h>

HardwareSerial mySerial(2);                                                        // RX, TX : CO2 sensor on pin 16 and 17
unsigned char hexdata[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; //Read the gas density command /Don't change the order

bool batteryControler;
int batteryLevel = -1;
bool isCharging = false;
bool fullCharge = false;
bool SDCardPresent = false;
bool recording = false;

TaskHandle_t Task1, Task2;
void TaskRecord(void *pvParameters);
void TaskReadCO2(void *pvParameters);

void Screen_begin();
void redraw();
long readCO2();
long CO2;

void initTime();

bool test_SDCard()
{
  /*File file = SD.open("/", 0);

  if (!file)
  {
    Serial.println("echec card");
    SDCardPresent = false;
  }
  else
  {
    SDCardPresent = true;
    file.close();
  }

  return SDCardPresent;*/

  SDCardPresent = SD.begin();
  return SDCardPresent;
}

long rec_time_start;
//char filename[22];

void setup()
{

  M5.begin();
  Wire.begin();
  M5.Power.begin();
  Screen_begin();
  Wire.begin();

  batteryControler = M5.Power.canControl();

  Serial.begin(9600);
  mySerial.begin(9600);

  test_SDCard();

  pinMode(25, OUTPUT); // for a quiet speaker

  if (batteryControler)
    Serial.println("battery controler");
  else
    Serial.println("no battery controler");

  initTime();

  xTaskCreatePinnedToCore(TaskReadCO2, "Task1", 20000, NULL, 1, &Task1, 0);
}

void loop()
{
  //mySerial.write(hexdata, 9);

  delay(10);
  static int count = 0; // mesure tous les 500ms
  count++;
  if (count % 100 == 0)
  {
    Serial.print("CO2 concentration: ");
    Serial.println(CO2);
  }

  redraw();
  M5.update();
}

char *DrawTime(int format);
File file;

void startrec()
{
  xTaskCreatePinnedToCore(TaskRecord, "Task2", 20000, NULL, 1, &Task2, 0);
}

void stoptrec()
{
  Serial.println("stop recording");
  vTaskDelete(Task2);
  file.close();
  recording = false;
}

void TaskRecord(void *pvParameters)
{
  char filename[25];
  sprintf(filename, "/%s.txt", DrawTime(1));

  Serial.print("star recording in ");
  Serial.println(filename);

  file = SD.open(filename, FILE_WRITE);

  if (!file)
  {
    Serial.println("SD error");
    vTaskDelete(NULL);
  }
  else
  {
    recording = true;

    file.printf("#%s\n", DrawTime(1));
    file.print("#Time(s),CO2 concentration(ppm)\n");

    unsigned long rec_time_start = millis();

    while (true)
    {
      delay(1000);
      int t = (int)((millis() - rec_time_start) / 1000);

      char buf[20];
      sprintf(buf, "%d,%ld\n", t, CO2);
      file.printf(buf);
      file.flush(); //Avoid loosing data if battery is low or card removed sauvagely
    }
  }
}

void TaskReadCO2(void *pvParameters)
{
  while (true)
  {
    mySerial.write(hexdata, 9);
    delay(10);
    CO2 = readCO2();
    delay(1000);
  }
}

long readCO2()
{
  long hi = 0, lo = 0, CO2 = -1;

  for (int i = 0; i < 9; i++)
  {

    if (mySerial.available() > 0)
    {

      int ch = mySerial.read();

      if (i == 2)
      {
        hi = ch;
      } //High concentration
      if (i == 3)
      {
        lo = ch;
      } //Low concentration
      if (i == 8)
      {
        CO2 = hi * 256 + lo; //CO2 concentration
      }
    }
  }

  return CO2;
}