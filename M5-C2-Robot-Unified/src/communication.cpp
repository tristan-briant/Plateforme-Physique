#include "Vrekrer_scpi_parser.h"

#define __TS_VERSION__ "V1.0"

extern bool LeftInput, RightInput;
void set_target_mm(float xl, float xr);
void set_speed(float speedl, float speedr);

bool isMoving();
bool getRightSensor();
bool getLeftSensor();

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    // Serial.print("Robot, ");
    // Serial.println(__TS_VERSION__);
    interface.print(F("Robot, "));
    interface.println(F(__TS_VERSION__));
}

void move(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    float newValueLeft, newValueRight;
    if (parameters.Size() == 0)
        return;
    if (parameters.Size() == 1)
    {
        newValueLeft = newValueRight = String(parameters[0]).toFloat();
    }
    if (parameters.Size() == 2)
    {
        newValueLeft = String(parameters[0]).toFloat();
        newValueRight = String(parameters[1]).toFloat();
    }

    set_target_mm(newValueLeft, newValueRight);
}

void speed(SCPI_C commands, SCPI_P parameters, Stream &interface)
{

    float newValueLeft, newValueRight;
    if (parameters.Size() == 0)
        return;
    if (parameters.Size() == 1)
    {
        newValueLeft = newValueRight = String(parameters[0]).toFloat() / 100.0f;
    }
    if (parameters.Size() == 2)
    {
        newValueLeft = String(parameters[0]).toFloat() / 100.0f;
        newValueRight = String(parameters[1]).toFloat() / 100.0f;
    }

    set_speed(newValueLeft, newValueRight);
}

void MovingStatut(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (isMoving())
        interface.println("1");
    else
        interface.println("0");
}

void sensor(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    char str[5];

    if (getLeftSensor())
        str[0] = '1';
    else
        str[0] = '0';

    str[1] = ',';

    if (getRightSensor())
        str[2] = '1';
    else
        str[2] = '0';

    /*interface.print(",");
    if (getRightSensor())
        interface.print("1");
    else
        interface.print("0");

    if (getLeftSensor())
        interface.print("1");
    else
        interface.print("0");
    interface.print(",");
    if (getRightSensor())
        interface.print("1");
    else
        interface.print("0");*/
    interface.println(str);
}

void initialize_SCPI()
{
    int param = 1;
    my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    my_instrument.RegisterCommand(F("MOVE"), &move);
    my_instrument.RegisterCommand(F("SPEED"), &speed);
    my_instrument.RegisterCommand(F("MOVING?"), &MovingStatut);
    my_instrument.RegisterCommand(F("SENSOR?"), &sensor);
}

void loopComunication(void *param)
{
    initialize_SCPI();
    while (true)
    {
        my_instrument.ProcessInput(Serial, "\n");

        delay(1);
    }
}
