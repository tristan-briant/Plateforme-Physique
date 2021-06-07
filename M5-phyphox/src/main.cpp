#include <Arduino.h>
#include "BluetoothSerial.h"
#include <M5Stack.h>
#include "utility/MPU9250.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_system.h>
#include <BLE2902.h>
#include "bt-util.h"
#include "pref-util.h"

MPU9250 IMU;

bool BMM150;
bool batteryControler;
int batteryLevel = -1;
bool isCharging = false;
bool fullCharge = false;
float angleX, angleY, angleZ;

bool bmm150_begin();
void bmm150_read_data(float *data);
void bmm150_offset_save(float *offset);
void bmm150_offset_load(float *offset);

void redraw();

float accData[4];
float gyrData[4];
float magData[4];

float accRaw[4];
float gyrRaw[4];
float magRaw[4];

float accBias[4] = {0, 0, 0, 0};
float accScale[4] = {1, 1, 1, 1};

float gyrBias[4] = {0, 0, 0, 0};
float magBias[4] = {0, 0, 0, 0};

TaskHandle_t TaskUI;
void TaskUIcode(void *pvParameters);
void magAverage(int step);

int brightness;

void setup()
{
  Serial.begin(9600);
  M5.begin();
  M5.Power.begin();
  Wire.begin();

  batteryControler = M5.Power.canControl();
  load_name();
  load_cal("magnetometer", magBias);
  load_cal("gyrometer", gyrBias);
  load_cal("accel_bias", accBias);
  load_cal("accel_scale", accScale);
  brightness = load_brightness();
  M5.lcd.setBrightness(brightness);

  pinMode(25, OUTPUT); // for a quiet speaker

  BLEinit();
  delay(100);

  //IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
  IMU.initMPU9250();

  BMM150 = bmm150_begin(); // true if magnetometer bmm150 present, AK8963 otherwise
  if (!BMM150)
    IMU.initAK8963(IMU.magCalibration);

  Serial.printf("%f   %f   %f\n", magBias[1], magBias[2], magBias[3]);

  xTaskCreatePinnedToCore(TaskUIcode, "Task1", 20000, NULL, 1, &TaskUI, 0);
}

void loop()
{
  static int count = 0;

  if (count++ % 1000 == 0)
  {
    batteryLevel = M5.Power.getBatteryLevel();
    isCharging = M5.Power.isCharging();
    fullCharge = M5.Power.isChargeFull();
  }

  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    IMU.readAccelData(IMU.accelCount);
    IMU.getAres();

    accRaw[1] = (float)IMU.accelCount[0] * IMU.aRes; // - accelBias[0];
    accRaw[2] = (float)IMU.accelCount[1] * IMU.aRes; // - accelBias[1];
    accRaw[3] = (float)IMU.accelCount[2] * IMU.aRes; // - accelBias[2];

    IMU.readGyroData(IMU.gyroCount);
    IMU.getGres();

    gyrRaw[1] = (float)IMU.gyroCount[0] * IMU.gRes;
    gyrRaw[2] = (float)IMU.gyroCount[1] * IMU.gRes;
    gyrRaw[3] = (float)IMU.gyroCount[2] * IMU.gRes;

    if (!BMM150)
    {
      IMU.readMagData(IMU.magCount);
      IMU.getMres();
      magRaw[1] = (float)IMU.magCount[1] * IMU.mRes;
      magRaw[2] = (float)IMU.magCount[0] * IMU.mRes;
      magRaw[3] = -(float)IMU.magCount[2] * IMU.mRes; // magnetometer up side down in case
    }
    else
    {
      float d[3];
      bmm150_read_data(d);
      magRaw[1] = d[0] * 10.0; // factor 10 to convert µT to mG
      magRaw[2] = d[1] * 10.0;
      magRaw[3] = d[2] * 10.0;
    }

    magAverage(1);

    float t = 0.000001 * micros();

    accData[0] = t;

    for (int i = 1; i < 4; i++)
      accData[i] = (accRaw[i] - accBias[i]) * accScale[i];

    gyrData[0] = t;
    for (int i = 1; i < 4; i++)
      gyrData[i] = (gyrRaw[i] - gyrBias[i]);

    magData[0] = t;
    for (int i = 1; i < 4; i++)
      magData[i] = (magRaw[i] - magBias[i]);

    static float tt = micros() / 1000000;
    angleX += gyrData[1] * (t - tt);
    angleY += gyrData[2] * (t - tt);
    angleZ += gyrData[3] * (t - tt);
    tt = t;

    if (BLEisConnected())
      BLEsendData();
  }

  delay(1);
}

void TaskUIcode(void *pvParameters)
{

  while (true)
  {
    redraw();

    delay(50);
    M5.update();
  }
}

void magAverage(int step)
{
  static float magMin[4], magMax[4];

  if (step == 0) //initialize
  {
    for (int i = 1; i < 4; i++)
      magMin[i] = magMax[i] = magRaw[i];
  }

  if (step == 1)
  {
    for (int i = 1; i < 4; i++)
    {
      if (magRaw[i] < magMin[i])
        magMin[i] = magRaw[i];
      if (magRaw[i] > magMax[i])
        magMax[i] = magRaw[i];
    }
  }

  if (step == 2)
    for (int i = 1; i < 4; i++)
      magBias[i] = 0.5 * (magMax[i] + magMin[i]);

  if (step == 3)
  {
    save_cal("magnetometer", magBias);
    //bmm150_offset_save(magBias);
  }
}

void reset_cal_data()
{
  for (int i = 0; i < 4; i++)
  {
    magBias[i] = accBias[i] = gyrBias[i] = 0;
  }
}