#include "Vrekrer_scpi_parser.h"

#define __TS_VERSION__ "V0.1"

extern float power;

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    Serial.print("Temperature Controller, ");
    Serial.println(__TS_VERSION__);
    interface.print(F("Temperature Controller, "));
    interface.println(F(__TS_VERSION__));
}

void setpower(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (parameters.Size() == 0)
        return;
    double newValue = String(parameters[0]).toFloat();

    Serial.println(parameters[0]);
    power = constrain(newValue, 0, 100);
}

void getpower(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(String(power, DEC));
}

void initialize_SCPI()
{
    int param = 1;
    my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    my_instrument.RegisterCommand(F("POWer"), &setpower);
    my_instrument.RegisterCommand(F("POWer?"), &getpower);
}

void loopComunication(void *param)
{
    initialize_SCPI();
    while (true)
    {
        my_instrument.ProcessInput(Serial, "\n");
        delay(10);
    }
}
