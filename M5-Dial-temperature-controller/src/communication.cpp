#include "PID_v1.h"
// #include <M5Stack.h>
#include "Vrekrer_scpi_parser.h"

#define __TC_VERSION__ "V0.2"

void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1);  //, PID pid2);
void load_tuning(PID *pid1); //, PID *pid2);

extern PID pid1;
extern double temp1, temp2, tset1, tset2;

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.print(F("Temperature Controller, "));
    interface.println(F(__TC_VERSION__));
}

void getTemp(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (parameters.Size() == 0 || String(parameters[0]).toInt() == 1)
        interface.printf("%.2f\n", temp1);
    else if (String(parameters[0]).toInt() == 2)
        interface.printf("%.2f\n", temp2);
}

void getTunings(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();

    PID *pidselect = &pid1;

    /*if (parameters.Size() > 0 && String(parameters[0]) == "2")
        pidselect = &pid2;
    else
        pidselect = &pid1;*/

    if (last_header == "GP?")
        interface.printf("%.5f\n", pidselect->GetKp());
    else if (last_header == "GD?")
        interface.printf("%.5f\n", pidselect->GetKd());
    else if (last_header == "GI?")
        interface.printf("%.5f\n", pidselect->GetKi());
    else if (last_header == "TSET?")
        interface.printf("%.2f\n", pidselect->GetSet());
    else if (last_header == "DIR?")
        interface.printf("%.2f\n", pidselect->GetDirection());
    else if (last_header == "MAX?")
        interface.printf("%.2f\n", pidselect->GetOutMax());
    else if (last_header == "MIN?")
        interface.printf("%.2f\n", pidselect->GetOutMin());
}

void setTunings(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = &pid1;

    if (parameters.Size() == 0)
        return;

    double newValue = String(parameters[0]).toFloat();

    if (last_header == "TSET")
    {
        tset1 = newValue;
        // save_value("PID1", "TSET", tset1);
        return;
    }
    else if (last_header == "MAX")
        pidselect->SetOutputLimits(pidselect->GetOutMin(), newValue);
    else if (last_header == "MIN")
        pidselect->SetOutputLimits(newValue, pidselect->GetOutMax());
    else if (last_header == "GP")
        pidselect->SetTunings(newValue, pidselect->GetKi(), pidselect->GetKd());
    else if (last_header == "GI")
        pidselect->SetTunings(pidselect->GetKp(), newValue, pidselect->GetKd());
    else if (last_header == "GD")
        pidselect->SetTunings(pidselect->GetKp(), pidselect->GetKi(), newValue);
    else if (last_header.startsWith("DIR"))
        pidselect->SetControllerDirection(newValue > 0 ? 1 : -1);
    save_tuning(pid1);
}

void setLimits(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = &pid1;

    Serial.println("set !");

    if (parameters.Size() == 0)
        return;

    double newValue = String(parameters[0]).toFloat();
    double max, min;

    if (last_header == "MAXI")
    {
        min = pidselect->GetOutMin();
        max = constrain(newValue, -100, 100);
    }
    else if (last_header == "MINI")
    {
        max = pidselect->GetOutMax();
        min = constrain(newValue, -100, 100);
    }
    else
        return;

    if (min >= max)
        return;

    pidselect->SetOutputLimits(min, max);
    save_tuning(pid1);
}

void getLimits(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = &pid1;

    Serial.println("get !");

    if (last_header == "MINI?")
        interface.printf("%.5f\n", pidselect->GetOutMin());
    else if (last_header == "MAXI?")
        interface.printf("%.5f\n", pidselect->GetOutMax());
}

void initialize_SCPI()
{
    my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    my_instrument.RegisterCommand(F("TEMP?"), &getTemp);
    
    my_instrument.SetCommandTreeBase(F("PID:"));
    my_instrument.RegisterCommand(F(":TSET"), &setTunings);
    my_instrument.RegisterCommand(F(":GP"), &setTunings);
    my_instrument.RegisterCommand(F(":GD"), &setTunings);
    my_instrument.RegisterCommand(F(":GI"), &setTunings);
    my_instrument.RegisterCommand(F(":DIR"), &setTunings);
    my_instrument.RegisterCommand(F(":MAX"), &setTunings);
    my_instrument.RegisterCommand(F(":MAX?"), &getTunings);
    my_instrument.RegisterCommand(F(":MIN"), &setTunings);
    my_instrument.RegisterCommand(F(":MIN?"), &getTunings);
    my_instrument.RegisterCommand(F(":GP?"), &getTunings);
    my_instrument.RegisterCommand(F(":GD?"), &getTunings);
    my_instrument.RegisterCommand(F(":GI?"), &getTunings);
    my_instrument.RegisterCommand(F(":DIR?"), &getTunings);
    my_instrument.RegisterCommand(F(":TSET?"), &getTunings);
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
