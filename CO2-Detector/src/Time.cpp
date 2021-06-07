#include <Wire.h>
#include "RTClib.h"

#define I2C_SDA_RTC 5
#define I2C_SCL_RTC 26

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void initTime()
{
    Wire.begin(I2C_SDA_RTC, I2C_SCL_RTC);

    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        abort();
    }

    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

DateTime get_time()
{
    Wire.begin(I2C_SDA_RTC, I2C_SCL_RTC);
    return rtc.now();
}

char *DrawTime(int format)
{
    static char buf[50];

    DateTime now = get_time();
    if (format == 0)
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    if (format == 1) // for filename
        sprintf(buf, "%04d-%02d-%02d-%02d-%02d-%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

    return buf;
}

void IncrementDateTime(int selection, int increment)
{ //selection 0=year 1=month....
    DateTime now = get_time();
    int y = now.year(), mo = now.month(), d = now.day(), h = now.hour(), mn = now.minute(), s = now.second();
    switch (selection)
    {
    case 0:
        y = constrain(y + increment, 2020, 2050);
        break;
    case 1:
        mo = constrain(mo + increment, 1, 12);
        break;
    case 2:
        d = constrain(d + increment, 1, 31);
        break;
    case 3:
        h = constrain(h + increment, 0, 23);
        break;
    case 4:
        mn = constrain(mn + increment, 0, 59);
        break;
    default:
        break;
    }

    rtc.adjust(DateTime(y, mo, d, h, mn, s));
}