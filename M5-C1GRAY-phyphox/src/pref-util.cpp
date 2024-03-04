#include <Preferences.h>

Preferences preferencesdata;

int name_number;

void reset_cal_data(); // dans le main

void load_cal(const char *name, float *data)
{
    if (preferencesdata.begin(name, true))
    {
        preferencesdata.getBytes("calibration", (uint8_t *)data, 4 * sizeof(float));
        preferencesdata.end();
    }
}

void save_cal(const char *name, float *data)
{
    if (preferencesdata.begin(name, false))
    {
        preferencesdata.putBytes("calibration", (uint8_t *)data, 4 * sizeof(float));
        preferencesdata.end();
    }
}

void save_name(int n)
{
    name_number = n;
    if (preferencesdata.begin("name", false))
    {
        preferencesdata.putInt("number", n);
        preferencesdata.end();
        Serial.printf("name saved: %d", n);
    }
}

void save_brightness(int n)
{
    if (preferencesdata.begin("brightness", false))
    {
        preferencesdata.putInt("bright", n);
        preferencesdata.end();
        Serial.printf("name saved: %d", n);
    }
}

int load_name()
{
    name_number = 0;
    if (preferencesdata.begin("name", true))
    {
        int n = preferencesdata.getInt("number");
        preferencesdata.end();
        name_number = n;
    }
    return name_number;
}

int load_brightness()
{
    if (preferencesdata.begin("brightness", true))
    {
        int n = preferencesdata.getInt("bright");
        preferencesdata.end();
        return n;
    }
    else
        return 128;
}

void increment_name(int inc = +1)
{
    int ndev = load_name() + inc;

    if (ndev >= 0 && ndev < 1000)
        save_name(ndev);
}

int increment_brightness(int inc = +1)
{
    extern int brightness;

    brightness += inc;

    if (brightness < 1)
        brightness = 1;
    if (brightness > 255)
        brightness = 255;

    save_brightness(brightness);
    return brightness;
}

char *name = new char[20];

char *getDeviceName()
{
    //char *name = new char[20];

    //int number = load_name();
    sprintf(name, "M5-SENSOR-%03d", name_number);
    return name;
}

void clear_all_data()
{
    if (preferencesdata.begin("magnetometer", false))
    {
        preferencesdata.clear();
        preferencesdata.end();
    }
    if (preferencesdata.begin("gyrometer", false))
    {
        preferencesdata.clear();
        preferencesdata.end();
    }
    if (preferencesdata.begin("accel_bias", false))
    {
        preferencesdata.clear();
        preferencesdata.end();
    }
    if (preferencesdata.begin("accel_scale", false))
    {
        preferencesdata.clear();
        preferencesdata.end();
    }

    reset_cal_data();
}

void clear_name()
{
    if (preferencesdata.begin("name", false))
    {
        preferencesdata.clear();
        preferencesdata.end();
    }
}