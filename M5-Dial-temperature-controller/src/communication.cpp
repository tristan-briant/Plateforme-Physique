#include "PID_v1.h"
// #include <M5Stack.h>
#include "Vrekrer_scpi_parser.h"

#define __TC_VERSION__ "V0.1"

void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1, PID pid2);
void load_tuning(PID *pid1, PID *pid2);

extern PID pid1, pid2;
extern double temp1, temp2, tset1, tset2;

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    Serial.println("titi");
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

    PID *pidselect = NULL;

    if (parameters.Size() > 0 && String(parameters[0]) == "2")
        pidselect = &pid2;
    else
        pidselect = &pid1;

    if (last_header == "GP?")
        interface.printf("%.5f\n", pidselect->GetKp());
    else if (last_header == "GD?")
        interface.printf("%.5f\n", pidselect->GetKd());
    else if (last_header == "GI?")
        interface.printf("%.5f\n", pidselect->GetKi());
    else if (last_header == "TSET?")
        interface.printf("%.2f\n", pidselect->GetSet());
}

void setTunings(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = NULL;

    if (parameters.Size() == 0)
        return;

    double newValue = String(parameters[0]).toFloat();

    if (parameters.Size() > 1 && String(parameters[1]) == "2")
        pidselect = &pid2;
    else
        pidselect = &pid1;

    if (last_header == "TSET")
    {
        if (pidselect == &pid1)
        {
            tset1 = newValue;
            save_value("PID1", "TSET", tset1);
        }
        else
        {
            tset2 = newValue;
            save_value("PID2", "TSET", tset2);
        }
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
    save_tuning(pid1, pid2);
}

void setLimits(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();
    PID *pidselect = NULL;

    if (parameters.Size() == 0)
        return;

    double newValue = String(parameters[0]).toFloat();

    if (parameters.Size() > 1 && String(parameters[1]) == "2")
        pidselect = &pid2;
    else
        pidselect = &pid1;

    double max, min;

    if (last_header == "OUTMAX")
    {
        min = pidselect->GetOutMin();
        max = constrain(newValue, -100, 100);
    }
    else if (last_header == "OUTMIN")
    {
        max = pidselect->GetOutMax();
        min = constrain(newValue, -100, 100);
    }
    else
        return;

    if (min >= max)
        return;

    pidselect->SetOutputLimits(min, max);
    /*save_value("PID1", "OUTMAX", pid1.GetOutMax());
    save_value("PID1", "OUTMIN", pid1.GetOutMin());
    save_value("PID2", "OUTMAX", pid2.GetOutMax());
    save_value("PID2", "OUTMIN", pid2.GetOutMin());*/

    save_tuning(pid1, pid2);
}

void getLimits(SCPI_C commands, SCPI_P parameters, Stream &interface) {}

void initialize_SCPI()
{
    int param = 1;
    my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    my_instrument.RegisterCommand(F("TEMP?"), &getTemp);
    my_instrument.SetCommandTreeBase(F("PID:"));
    my_instrument.RegisterCommand(F(":GP"), &setTunings);
    my_instrument.RegisterCommand(F(":GD"), &setTunings);
    my_instrument.RegisterCommand(F(":GI"), &setTunings);
    my_instrument.RegisterCommand(F(":DIRection"), &setTunings);
    my_instrument.RegisterCommand(F(":OUTMAX"), &setLimits);
    my_instrument.RegisterCommand(F(":OUTMIN"), &setLimits);

    my_instrument.RegisterCommand(F(":GP?"), &getTunings);
    my_instrument.RegisterCommand(F(":GD?"), &getTunings);
    my_instrument.RegisterCommand(F(":GI?"), &getTunings);
    my_instrument.RegisterCommand(F(":TSET?"), &getTunings);
    my_instrument.RegisterCommand(F(":OUTMAX?"), &getLimits);
    my_instrument.RegisterCommand(F(":OUTMIN?"), &getLimits);

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
        delay(10);
    }
}

/*void loopComunication(void *param)
{
    while (true)
    {
        if (Serial.available())
        {
            int error = 1;
            do
            {
                const int16_t BUFFER_SIZE = 100;
                String str(BUFFER_SIZE);
                str = Serial.readStringUntil('\n');
                str.toUpperCase();

                if (str == "TEMP1?")
                    Serial.printf("%.2f\n", temp1);
                else if (str == "TEMP2?")
                    Serial.printf("%.2f\n", temp2);
                else if (str == "*IDN?")
                    Serial.printf("Temperature Controller %s\n", __TC_VERSION__);
                else if (str.startsWith("PID"))
                {
                    PID *pidselect = NULL;

                    if (str.startsWith("PID1:"))
                        pidselect = &pid1;
                    else if (str.startsWith("PID2:"))
                        pidselect = &pid2;
                    else
                        break;
                    str = str.substring(5);
                    if (str == "GP?")
                        Serial.printf("%.5f\n", pidselect->GetKp());
                    else if (str == "GD?")
                        Serial.printf("%.5f\n", pidselect->GetKd());
                    else if (str == "GI?")
                        Serial.printf("%.5f\n", pidselect->GetKi());
                    else if (str == "TSET?")
                        Serial.printf("%.2f\n", pidselect->GetSet()); // pidselect == &pid1 ? tset1 : tset2);

                    else if (str.startsWith("GP "))
                    {
                        float x = str.substring(3).toFloat();
                        pidselect->SetTunings(x, pidselect->GetKi(), pidselect->GetKd());
                        save_tuning(pid1, pid2);
                    }
                    else if (str.startsWith("GI "))
                    {
                        float x = str.substring(3).toFloat();
                        pidselect->SetTunings(pidselect->GetKp(), x, pidselect->GetKd());
                        save_tuning(pid1, pid2);
                    }
                    else if (str.startsWith("GD "))
                    {
                        float x = str.substring(3).toFloat();
                        pidselect->SetTunings(pidselect->GetKp(), pidselect->GetKi(), x);
                        save_tuning(pid1, pid2);
                    }
                    else if (str.startsWith("TSET "))
                    {
                        float x = str.substring(5).toFloat();
                        if (pidselect == &pid1)
                        {
                            tset1 = x;
                            save_value("PID1", "TSET", tset1);
                        }
                        else
                        {
                            tset2 = x;
                            save_value("PID2", "TSET", tset2);
                        }
                    }
                }

                error = 0;
            } while (false);

            if (error)
                Serial.println("syntaxe error");
        }

        delay(10);
    }
}*/