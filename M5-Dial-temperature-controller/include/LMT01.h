#include <M5unified.h>

volatile int16_t TemperatureCounts; // temperature in LMT01 counts

void ISRCorrection()
{
    const unsigned long COUNTER_TIMEOUT = 50;

    static int16_t TempISRCount;       // counter used in ISR
    static unsigned long milliseconds; // time in milliseconds of previous pulse
    static unsigned long microseconds; // time in milliseconds of previous pulse

    unsigned long current = millis();
    unsigned long difference = current - milliseconds;

    unsigned long current_us = micros();
    unsigned long difference_us = current_us - microseconds;

    if (difference > COUNTER_TIMEOUT)
    {

        // previous pulse was too long ago, lets start counting new measure
        TemperatureCounts = TempISRCount;
        TempISRCount = 1;
        //  gpio_set_level(GPIO_NUM_13, 1);
        // gpio_set_level(GPIO_NUM_13, 0);
    }
    else
    {
        // just counting
        if (difference_us > 5) // to avoid false count
        {
            TempISRCount++;
            /*if (difference_us > 15)
                TempISRCount++;*/
            microseconds = current_us;
        }
        // gpio_set_level(GPIO_NUM_13, 1);
        // gpio_set_level(GPIO_NUM_13, 0);
    }
    // store current time
    milliseconds = current;
}

void start_LMT01(int pin_LMT01)
{
    attachInterrupt(pin_LMT01, ISRCorrection, FALLING);
}

void init_LMT01(int pin_LMT01)
{
    attachInterrupt(pin_LMT01, ISRCorrection, FALLING);
}

float get_temperature1()
{
    static int c = 0;
    c++;
    return TemperatureCounts + c;
}

volatile float get_temperature(int pin_LMT01)
{
    return TemperatureCounts / 16.0 - 50.0;
}