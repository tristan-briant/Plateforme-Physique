#include "PID_v1.h"
// #include <M5Stack.h>
#include "Vrekrer_scpi_parser.h"

#define __TC_VERSION__ "V0.1"

void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1);  //, PID pid2);
void load_tuning(PID *pid1); //, PID *pid2);

extern PID pid1;//, pid2;
extern double xact, xset;

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    //Serial.println("titi");
    interface.print(F("Temperature Controller, "));
    interface.println(F(__TC_VERSION__));
}

void getPos(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (parameters.Size() == 0 || String(parameters[0]).toInt() == 1)
        interface.printf("%.2f\n", xact);
    //else if (String(parameters[0]).toInt() == 2)
    //    interface.printf("%.2f\n", temp2);
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
    else if (last_header == "XSET?")
        interface.printf("%.2f\n", pidselect->GetSet());
}

void setTunings(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = &pid1;

    if (parameters.Size() == 0)
        return;

    double newValue = String(parameters[0]).toFloat();

    /*if (parameters.Size() > 1 && String(parameters[1]) == "2")
        pidselect = &pid2;
    else
        pidselect = &pid1;*/

    if (last_header == "XSET")
    {
        if (pidselect == &pid1)
        {
            xset = newValue;
            save_value("PID1", "XSET", xact);
        }
        /*else
        {
            tset2 = newValue;
            save_value("PID2", "TSET", tset2);
        }*/
        return;
    }
    else if (last_header == "GP")
        pidselect->SetTunings(newValue, pidselect->GetKi(), pidselect->GetKd());
    else if (last_header == "GI")
        pidselect->SetTunings(pidselect->GetKp(), newValue, pidselect->GetKd());
    else if (last_header == "GD")
        pidselect->SetTunings(pidselect->GetKp(), pidselect->GetKi(), newValue);
    else if (last_header.startsWith("DIR"))
        pidselect->SetControllerDirection(newValue > 0 ? 1 : -1);
    save_tuning(pid1); //, pid2);
}

void setLimits(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = &pid1;

    if (parameters.Size() == 0)
        return;

    double newValue = String(parameters[0]).toFloat();

    double max, min;
    Serial.println("setlimit");

    if (last_header == "MAX")
    {
        min = pidselect->GetOutMin();
        max = constrain(newValue, -100, 100);
    }
    else if (last_header == "MIN")
    {
        max = pidselect->GetOutMax();
        min = constrain(newValue, -100, 100);
    }
    else
        return;

    if (min >= max)
        return;

    pidselect->SetOutputLimits(min, max);
    save_tuning(pid1); //, pid2);
}

void getLimits(SCPI_C commands, SCPI_P parameters, Stream &interface) {
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = &pid1;

    //if (last_header == "MAX?")
      interface.printf("%.5f\n", pidselect->GetOutMax());
    
    //Felse if (last_header == "MIN?")
        interface.printf("%.5f\n", pidselect->GetOutMin());
}

void initialize_SCPI()
{
    int param = 1;
    my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    my_instrument.RegisterCommand(F("X?"), &getPos);

    my_instrument.SetCommandTreeBase(F("PID:"));
    my_instrument.RegisterCommand(F(":GP"), &setTunings);
    my_instrument.RegisterCommand(F(":GD"), &setTunings);
    my_instrument.RegisterCommand(F(":GI"), &setTunings);
    my_instrument.RegisterCommand(F(":DIRection"), &setTunings);
    my_instrument.RegisterCommand(F(":XSET"), &setTunings);

    my_instrument.RegisterCommand(F(":MAX"), &setLimits);
    my_instrument.RegisterCommand(F(":MIN"), &setLimits);

    my_instrument.RegisterCommand(F(":GP?"), &getTunings);
    my_instrument.RegisterCommand(F(":GD?"), &getTunings);
    my_instrument.RegisterCommand(F(":GI?"), &getTunings);
    my_instrument.RegisterCommand(F(":XSET?"), &getTunings);
    
    my_instrument.RegisterCommand(F(":MINmax?"), &getLimits);
    //my_instrument.RegisterCommand(F(":MAX?"), &getLimits);

    /*my_instrument.RegisterCommand(F(":BRIGhtness?"), &GetBrightness);
    my_instrument.RegisterCommand(F(":BRIGhtness:INCrease"), &IncDecBrightness);
    my_instrument.RegisterCommand(F(":BRIGhtness:DECrease"), &IncDecBrightness);*/
}

void loopComunication(void *param)
{
    initialize_SCPI();
    while (true)
    {
        my_instrument.ProcessInput(Serial, "\n");
        delay(100);
    }
}

