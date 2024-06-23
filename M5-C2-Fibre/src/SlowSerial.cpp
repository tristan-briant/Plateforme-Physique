#include "M5Core2.h"
#include "SlowSerial.h"

int SlowSerial::write(char b)
{
    static long nextT = 0;
    static int k = -1; // numéro du bit
    static long Periode = 1e6 / comSpeed;

    if (comSpeed >= 300) // Si la vitesse est grande on utilise le software serial
    {
        Serial2.write(b);
        delay(100);
        return 1;
    }

    long t = micros();

    if (t > nextT + 2 * Periode) // il y a eu un bug on reset
        k = -1;

    if (t > nextT) // Next bit
    {
        if (k < 0)
        {
            Periode = 1e6 / comSpeed;
            gpio_set_level(txpin, HIGH); // bit d'amorçe
            M5.Axp.SetLed(1);
            nextT = t + Periode;
            k = 0;
            return 0;
        }

        if (k < 8)
        {
            bool bit = !(b & 1 << k);
            gpio_set_level(txpin, bit); // next bit
            M5.Axp.SetLed(bit);
            nextT = nextT + Periode;
            k++;
            return 0;
        }
        else
        {
            
            gpio_set_level(txpin, LOW);
            M5.Axp.SetLed(0);

            k = -1;
            nextT = nextT + 10 * Periode;
            return 1;
        }
    }

    return 0;
}

char SlowSerial::read()
{
    static long nextT = 0;
    static int k = -1; // numéro du bit
    static long Periode = 1e6 / comSpeed;
    static char c;

    if (comSpeed >= 300)
    {
        if (Serial2.available())
            return Serial2.read();
        else
            return 0;
    }

    long t = micros();

    if (t > nextT + 2 * Periode) // il y a eu un bug on reset
        k = -1;

    if (k < 0)
    {
        Periode = 1e6 / comSpeed;
        c = 0;
        bool inputB = gpio_get_level(rxpin);
        // M5.Axp.SetLed(inputB);

        if (inputB == LOW) // attend un front montant
            return 0;
        else // On a le front on démarre la lecture
        {
            k = 0;
            nextT = t + Periode + Periode / 4;
            return 0;
        }
    }

    if (t > nextT) // next bit
    {
        bool inputB = gpio_get_level(rxpin);
        M5.Axp.SetLed(inputB);

        if (k < 8)
        {
            if (inputB == LOW)
                c += 1 << k;
            nextT = nextT + Periode;
            k++;
            return 0;
        }
        else
        {
            k = -1;
            return c;
        }
    }

    return 0;
}

void SlowSerial::begin(long baud, int RXpin, int TXpin, bool inv)
{
    comSpeed = baud;
    rxpin = (gpio_num_t)RXpin;
    txpin = (gpio_num_t)TXpin;
    inverse = inv;

    if (comSpeed < 300)
    {
        Serial2.end();
        if (rxpin > 0)
            pinMode(rxpin, INPUT);
        if (txpin > 0)
            pinMode(txpin, OUTPUT);

        gpio_set_level(txpin, 0);
    }
    else
    {
        // pinMode(rxpin, INPUT);
        // pinMode(txpin, OUTPUT);
        Serial2.begin(baud, SERIAL_8N1, rxpin, txpin, inv);
        // Serial2.begin(baud, SERIAL_8N1, 19, 27, true);
    }
}