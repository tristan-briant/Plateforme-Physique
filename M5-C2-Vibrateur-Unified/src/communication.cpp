#include "Vrekrer_scpi_parser.h"
#include <M5Unified.h>

#define __TC_VERSION__ "V0.1"

extern float freq, amp, offsetTarget;
extern bool needToRedraw;

SCPI_Parser my_instrument;

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.print(F("Oscilator, "));
    interface.println(F(__TC_VERSION__));
}

void getParam(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String last_header = String(commands.Last());
    last_header.toUpperCase();

    if (last_header == "FREQ?")
        interface.printf("%.2f\n", freq);
    if (last_header == "AMP?")
        interface.printf("%.2f\n", freq);
    if (last_header == "OFF?")
        interface.printf("%.2f\n", offsetTarget);
}

void setParam(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (parameters.Size() == 0)
        return;

    String last_header = String(commands.Last());
    last_header.toUpperCase();

    float newValue = String(parameters[0]).toFloat();

    if (last_header == "FREQ")
        freq = newValue;
    if (last_header == "AMP")
        amp = newValue;
    if (last_header == "OFF")
        offsetTarget = newValue;
    needToRedraw = true;
}

void screenShot(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    int image_height = M5.Lcd.height();
    int image_width = M5.Lcd.width();

    if (parameters.Size() == 0)
        return;

    int line = String(parameters[0]).toInt();

    /*if (line == 0)
        Serial.printf("P3\n%d %d\n255\n", image_width, image_height);*/

    // for (int j = 0; j < image_height; j++){
    int j = line;

    RGBColor data[320];
    M5.Lcd.readRectRGB(0,j,320,1,data);

    for (int i = 0; i < image_width; i++)
    {
        //RGBColor color = M5.Lcd.readPixelRGB(i, j);
        Serial.write(data[i].r);
        Serial.write(data[i].g);
        Serial.write(data[i].b);
        //Serial.printf("%d %d %d\n", data[i].r, data[i].g, data[i].b);
    }
    //Serial.println();

}

void initialize_SCPI()
{
    int param = 1;
    my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    my_instrument.RegisterCommand(F("AMP?"), &getParam);
    my_instrument.RegisterCommand(F("FREQ?"), &getParam);
    my_instrument.RegisterCommand(F("OFF?"), &getParam);

    my_instrument.RegisterCommand(F("AMP"), &setParam);
    my_instrument.RegisterCommand(F("FREQ"), &setParam);
    my_instrument.RegisterCommand(F("OFF"), &setParam);
    my_instrument.RegisterCommand(F("SS?"), &screenShot);
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
