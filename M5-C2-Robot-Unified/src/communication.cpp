#include "Vrekrer_scpi_parser.h"

#define __TS_VERSION__ "V0.1"

extern bool LeftInput, RightInput;
void set_target_mm(float xr, float xl);
void set_speed(float speedr, float speedl);

bool isMoving();
bool getRightSensor();
bool getLeftSensor();

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    //Serial.print("Robot, ");
    //Serial.println(__TS_VERSION__);
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
        newValueRight = String(parameters[0]).toFloat();
        newValueLeft = String(parameters[1]).toFloat();
    }

    set_target_mm(newValueRight, newValueLeft);
}

void speed(SCPI_C commands, SCPI_P parameters, Stream &interface)
{

    float newValueLeft, newValueRight;
    if (parameters.Size() == 0)
        return;
    if (parameters.Size() == 1)
    {
        newValueLeft = newValueRight = String(parameters[0]).toFloat()/100.0f;
    }
    if (parameters.Size() == 2)
    {
        newValueRight = String(parameters[0]).toFloat() / 100.0f;
        newValueLeft = String(parameters[1]).toFloat() / 100.0f;
    }

    interface.println(newValueRight);
     interface.println(newValueLeft);
    set_speed(newValueRight, newValueLeft);
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
    if (getRightSensor())
        interface.print("1,");
    else
        interface.print("0,");
    if (getLeftSensor())
        interface.println("1");
    else
        interface.println("0");
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

        /*if (send_message_flag)
        {
            Serial.println("DONE");
            send_message_flag = false;
        }*/
        delay(10);
    }
}
