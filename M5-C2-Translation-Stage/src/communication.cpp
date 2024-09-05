#include "Vrekrer_scpi_parser.h"

#define __TS_VERSION__ "V0.1"

extern double x_target, x_position;
// void set_speed(double speed);

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    Serial.print("Temperature Controller, ");
    Serial.println(__TS_VERSION__);
    interface.print(F("Temperature Controller, "));
    interface.println(F(__TS_VERSION__));
}

void goto_position(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (parameters.Size() == 0)
        return;
    double newValue = String(parameters[0]).toFloat();

    x_target = newValue;
}

void move(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (parameters.Size() == 0)
        return;
    double newValue = String(parameters[0]).toFloat();

    x_target += newValue;
}

void stop(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    x_target = x_position;
}

void setposition(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    double origine = 0;

    if (parameters.Size() > 0)
        origine = String(parameters[0]).toFloat();

    x_position = x_target = origine;
}

void getposition(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(String(x_position, DEC));
}

void initialize_SCPI()
{
    int param = 1;
    my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    my_instrument.RegisterCommand(F("GOTO"), &goto_position);
    my_instrument.RegisterCommand(F("MOVE"), &move);
    my_instrument.RegisterCommand(F("STOP"), &stop);
    my_instrument.RegisterCommand(F("POSition"), &setposition);
    my_instrument.RegisterCommand(F("POSition?"), &getposition);
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
