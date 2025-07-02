#include <M5Unified.h>
#include <Button.h>

extern float FREQ_MAX, AMPL_MAX;

enum ModeRun
{
    SINUS,
    MOTOR_BREAK,
    MOTOR_RELEASED,
    PULSE // not used
};
extern ModeRun mode_run;

int selection = 0;
const int NSELECTION = 3;

int colorSelection = DARKGREEN;
int colorText = WHITE;
int colorBackGround = BLACK;

extern float freq, amp, offset, power;
extern float offsetTarget, offsetCal;

M5Canvas img(&M5.Lcd);

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3;
}

uint16_t color565(uint8_t grey)
{
    return (grey & 0xF8) << 8 | (grey & 0xFC) << 3 | grey >> 3;
}

void draw(int part = -1)
{
    int ypos[] = {50, 100, 150, 200};
    int xpos;

    static long told = millis();
    long t = millis();

    if (part < 0 || part == 0) /// Draw the freq frame
    {

        img.fillRect(5, 0, 320 - 10, 50, selection == 0 ? colorSelection : BLACK);
        img.drawRect(5, 0, 320 - 10, 50, colorSelection);

        img.setTextColor(colorText);
        img.setTextDatum(BL_DATUM);
        img.setFont(&FreeSans18pt7b);
        img.drawString("Freq ", 10, 50);
        img.drawString("Hz", 250, 50);
        img.setTextDatum(BR_DATUM);
        img.setFont(&FreeSansBold24pt7b);
        img.drawFloat(freq, 2, 240, 50 + 2);

        img.pushSprite(0, 0);
    }
    if (part < 0 || part == 1) /// Draw the Amplitude frame
    {
        img.fillRect(5, 0, 320 - 10, 50, selection == 1 ? colorSelection : BLACK);
        img.drawRect(5, 0, 320 - 10, 50, colorSelection);

        img.setTextColor(colorText);
        img.setTextDatum(BL_DATUM);
        img.setFont(&FreeSans18pt7b);
        img.drawString("Ampl ", 10, 50);
        img.drawString("mm", 250, 50);
        img.setTextDatum(BR_DATUM);
        img.setFont(&FreeSansBold24pt7b);
        img.drawFloat(amp, 1, 240, 50 + 2);

        img.pushSprite(0, 50);
    }
    if (part < 0 || part == 2) /// Draw the Offset frame
    {
        img.fillRect(5, 0, 320 - 10, 50, selection == 2 ? colorSelection : BLACK);
        img.drawRect(5, 0, 320 - 10, 50, colorSelection);

        img.setTextColor(colorText);
        img.setTextDatum(BL_DATUM);
        img.setFont(&FreeSans18pt7b);
        img.drawString("Offset ", 10, 50);
        img.drawString("mm", 250, 50);
        img.setTextDatum(BR_DATUM);
        img.setFont(&FreeSansBold24pt7b);
        img.drawFloat(offsetTarget - offsetCal, 1, 240, 50 + 2);

        img.pushSprite(0, 100);
    }
    if (part < 0 || part == 3) /// Draw the On/Off frame
    {
        // img.fillRect(5, 0, 320 - 10, 50, selection == 3 ? colorSelection : BLACK);
        // img.drawRect(5, 0, 320 - 10, 50, colorSelection);
        img.fillSprite(BLACK);
        img.setTextColor(colorText);
        img.setFont(&FreeSansBold12pt7b);
        img.setTextDatum(CC_DATUM);
        // img.setTextColor(mode_run == SINUS ? colorText : DARKGREY);
        // img.setFont(mode_run == SINUS ? &FreeSansBold12pt7b : &FreeSans12pt7b);
        img.fillRoundRect(320 / 6 - 50, 2, 100, 46, 15, color565(60));
        img.drawRoundRect(320 / 6 - 50, 2, 100, 46, 15, colorSelection);
        img.drawString(mode_run == SINUS ? "Off" : "On", 320 / 6, 25);
        img.setTextDatum(CC_DATUM);
        // img.setTextColor(mode_run == MOTOR_BREAK ? colorText : DARKGREY);
        // img.setFont(mode_run == MOTOR_BREAK ? &FreeSansBold12pt7b : &FreeSans12pt7b);
        img.fillRoundRect(160 - 50, 2, 100, 46, 15, color565(60));
        img.drawRoundRect(160 - 50, 2, 100, 46, 15, colorSelection);
        img.drawString("Pulse", 160, 25);
        img.setTextDatum(CL_DATUM);
        // img.setTextColor(mode_run == MOTOR_RELEASED ? colorText : DARKGREY);
        // img.setFont(mode_run == MOTOR_RELEASED ? &FreeSansBold12pt7b : &FreeSans12pt7b);
        img.fillRoundRect(320 * 5 / 6 - 50, 2, 100, 46, 15, color565(60));
        img.drawRoundRect(320 * 5 / 6 - 50, 2, 100, 46, 15, colorSelection);
        if (mode_run == MOTOR_RELEASED)
            img.drawString("Lock", 160 + 80, 25);
        else
            img.drawString("Free", 160 + 80, 25);

        img.pushSprite(0, 150);
    }
    if (part < 0)
    {
        img.fillSprite(BLACK);
        img.setTextColor(colorText);
        // img.setFont(&FreeSans12pt7b);
        img.setTextDatum(BC_DATUM);
        img.setFont(&FreeSansBold18pt7b);
        img.fillRoundRect(320 / 6 - 50, 2, 100, 100, 15, color565(30));
        img.drawRoundRect(320 / 6 - 50, 2, 100, 100, 15, color565(100, 0, 0));
        img.drawString("-", 320 / 6, 38);

        img.fillRoundRect(320 * 5 / 6 - 50, 2, 100, 100, 15, color565(30));
        img.drawRoundRect(320 * 5 / 6 - 50, 2, 100, 100, 15, color565(100, 0, 0));
        img.drawString("+", 320 * 5 / 6, 38);

        img.pushSprite(0, 200);
    }

    Serial.printf("%6d  %6d\n", t - millis(), t - told);
    told = millis();

    return;
}

void TaskGUI(void *pvParameters)
{
    Button buttonFreqSelec(0, 0, 320, 50);
    Button buttonAmpSelec(0, 50, 320, 50);
    Button buttonOffsetSelec(0, 100, 320, 50);

    Button buttonOn(0, 150, 107, 50);
    Button buttonOff(107, 150, 107, 50);
    Button buttonFree(214, 150, 106, 50); //, color565(60), colorSelection, "free");

    // Button buttonSelect(0, 200, 107, 50);
    Button buttonMinus(0, 200, 107, 50);
    Button buttonPlus(214, 200, 106, 50);

    img.createSprite(320, 50); // Create a 320x240 canvas

    // buttonFree.draw();

    draw();
    delay(50);

    while (true)
    {
        float inc = 0;

        M5.update();

        if (buttonOffsetSelec.pressedFor(2000))
        {
            offsetCal = offsetTarget;
            draw();
        }
        else
        {
            if (M5.BtnA.wasPressed() || (M5.BtnA.pressedFor(300)) || buttonMinus.wasPressed() || (buttonMinus.pressedFor(300)))
                inc = -1;
            if (M5.BtnA.pressedFor(1000) || buttonMinus.pressedFor(1000))
                inc = -10;
            if (M5.BtnA.pressedFor(3000) || buttonMinus.pressedFor(3000))
                inc = -100;
            if (M5.BtnC.wasPressed() || (M5.BtnC.pressedFor(300)) || buttonPlus.wasPressed() || (buttonPlus.pressedFor(300)))
                inc = +1;
            if (M5.BtnC.pressedFor(1000) || buttonPlus.pressedFor(1000))
                inc = +10;
            if (M5.BtnC.pressedFor(3000) || buttonPlus.pressedFor(3000))
                inc = +100;
        }

        if (buttonFreqSelec.isFlicking())
            inc = Button::deltaFlickedH();
        if (buttonAmpSelec.isFlicking() || buttonOffsetSelec.isFlicking())
            inc = Button::deltaFlickedH() / 10.0;

        if (buttonFreqSelec.wasPressed())
        {
            selection = 0;
            draw();
        }
        if (buttonAmpSelec.wasPressed())
        {
            selection = 1;
            draw();
        }
        if (buttonOffsetSelec.wasPressed())
        {
            selection = 2;
            draw();
        }

        if (buttonOn.wasPressed())
        {
            if (mode_run == ModeRun::SINUS)
                mode_run = ModeRun::MOTOR_BREAK;
            else
                mode_run = ModeRun::SINUS;
            draw(3);
        }
        if (buttonOff.isPressed())
        {
            mode_run = ModeRun::PULSE;
            draw(3);
        }
        if (buttonFree.wasPressed())
        {
            if (mode_run == ModeRun::MOTOR_RELEASED)
                mode_run = ModeRun::MOTOR_BREAK;
            else
                mode_run = ModeRun::MOTOR_RELEASED;
            draw(3);
        }

        if (inc != 0)
        {
            switch (selection)
            {
            case 0:
                freq = constrain(freq + inc * 0.01, 0, FREQ_MAX);
                break;
            case 1:
                amp = constrain(amp + constrain(inc, -2, +2) * 0.1, 0, AMPL_MAX);
                break;
            case 2:
                offsetTarget += constrain(inc, -1, +1) * 0.2;
                break;
            }
            draw(selection);
        }
        delay(1);
    }
}
