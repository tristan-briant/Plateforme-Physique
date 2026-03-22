#include <M5Unified.h>
#include <Button.h>
#include "PID_v1.h"

extern float turn;
extern float turn_frac, turnTotal, turn_frac0;
extern float turnTotal_avg;
extern const float epsilon;

extern bool outputEnable;

extern float I, Q;
extern double xact, xset;
extern long t_loop_us;
extern double output, outputGUI, res;
extern PID pid1;

void save_tuning(PID pid1);

void drawCardran(int x, int y, int width, float value1, float value2, float value3 = 0)
{
    static bool first_time = true;
    static M5Canvas canvasCadran(&M5.Lcd);

    int r1, r2, r;
    float cosin, sinus;

    float angle1 = -value1 / 10 * 2 * PI + PI / 2;
    float angle2 = -value2 / 10 * 2 * PI + PI / 2;

    static float angle1_old = 0, angle2_old = 0;
    int x0 = width / 2, y0 = width / 2;
    r = width / 2;

    int color_main = color565(20);

    if (first_time)
    {
        canvasCadran.createSprite(width, width);

        first_time = false;
    }

    r1 = r * 0.9, r2 = 0.98 * r;
    const int N_TICKS = 10;

    canvasCadran.fillCircle(x0, y0, r, color_main);

    if (value3 >= 0)
        canvasCadran.fillArc(x0, y0, r1, r2, -90, value3 / 100 * 90 - 90, RED);
    else
        canvasCadran.fillArc(x0, y0, r1, r2, value3 / 100 * 90 - 90, -90, BLUE);

    for (int i = 0; i < N_TICKS; i++)
    {
        cosin = cos(2 * PI * i / N_TICKS + PI / 2);
        sinus = sin(2 * PI * i / N_TICKS + PI / 2);
        canvasCadran.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, WHITE);
    }
    canvasCadran.setTextDatum(TC_DATUM);
    canvasCadran.setFont(&FreeSans9pt7b);
    canvasCadran.drawFloat(xact, 1, width / 2, width / 2 + 5);

    r1 = 0.1 * r, r2 = 0.8 * r;
    cosin = cos(angle2_old);
    sinus = sin(angle2_old);
    canvasCadran.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, color_main);
    cosin = cos(angle2);
    sinus = sin(angle2);
    canvasCadran.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, RED);

    r1 = 0.35 * r, r2 = 0.7 * r;
    cosin = cos(angle1_old);
    sinus = sin(angle1_old);
    canvasCadran.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, color_main);
    cosin = cos(angle1);
    sinus = sin(angle1);
    canvasCadran.drawLine(x0 + cosin * r1, y0 - sinus * r1, x0 + cosin * r2, y0 - sinus * r2, GREEN);

    angle1_old = angle1;
    angle2_old = angle2;

    canvasCadran.pushSprite(x, y);
}

int selection = 1;
int mainColor = color565(220, 100, 0);
int colorText = WHITE;

bool paramValueChanged = false;  // can be used outside GUI.cpp (in communication.cpp to update new param values) 

void draw_frame(String name, float value, int x, int y, bool selected, int precision = 1)
{
    static bool first_time = true;
    static M5Canvas img(&M5.Lcd);

    if (first_time)
    {
        img.createSprite(160, 50);
        first_time = false;
    }

    img.fillRect(5, 0, 150, 50, selected ? mainColor : BLACK);
    img.drawRect(5, 0, 150, 50, mainColor);

    img.setTextColor(colorText);
    img.setTextDatum(BL_DATUM);
    img.setFont(&FreeSans18pt7b);
    // img.setFont(&FreeSansBold24pt7b);
    img.drawString(name, 10, 50);
    // img.drawString("t/s", 250, 50);
    img.setTextDatum(BR_DATUM);
    // img.setFont(&FreeSansBold24pt7b);
    // int precision = 1; // fabs(targetSpeed) > 9.99 ? 1 : 2;
    img.drawFloat(value, precision, 145, 50);

    img.pushSprite(x, y);
}

void draw(int part = -1)
{
    static bool first_time = true;
    static M5Canvas img(&M5.Lcd);

    if (first_time)
    {
        img.createSprite(100, 100);
        first_time = false;
    }

    int ypos[] = {50, 100, 150, 200};
    int xpos;
    char str[20];

    static long told = millis();
    long t = millis();

    if (part < 0 || part == 0) /// Draw the parameter frame
    {
        /*************************** Parameter ***************************************/

        draw_frame("P", pid1.GetKp(), 0, 0, selection == 1, 1);
        draw_frame("I", pid1.GetKi(), 0, 50, selection == 2, 1);
        draw_frame("D", pid1.GetKd(), 0, 100, selection == 3, 1);

        draw_frame("Max", pid1.GetOutMax(), 160, 0, selection == 4, 0);
        draw_frame("x", xset, 160, 50, selection == 5, 1);
    }
    if (part < 0 || part == 3) /// Draw the On/Off frame
    {
        img.fillSprite(BLACK);
        // if (mode_run == MODE_TURN)
        {
            img.fillRoundRect(5, 5, 90, 90, 15, outputEnable ? mainColor : color565(60));
            img.drawRoundRect(5, 5, 90, 90, 15, mainColor);
            img.setTextDatum(CC_DATUM);
            img.setTextColor(WHITE);
            img.setFont(&FreeSansBold12pt7b);
            img.drawString(outputEnable ? "OFF" : "ON", 50, 50);
        }
        img.pushSprite(220, 100);
    }
    if (part < 0 || part == 4) /// Draw the +/- frame
    {
        img.fillSprite(BLACK);
        img.setTextColor(colorText);
        // img.setFont(&FreeSans12pt7b);
        img.setTextDatum(BC_DATUM);

        img.setFont(&FreeSansBold18pt7b);
        img.fillRoundRect(0, 2, 100, 100, 15, color565(30));
        img.drawRoundRect(0, 2, 100, 100, 15, color565(100, 0, 0));
        img.drawString("-", 50, 38);
        img.pushSprite(10, 200);

        img.setFont(&FreeSansBold18pt7b);
        img.fillRoundRect(0, 2, 100, 100, 15, color565(30));
        img.drawRoundRect(0, 2, 100, 100, 15, color565(100, 0, 0));
        img.drawString("+", 50, 38);
        img.pushSprite(210, 200);
    }

    // Serial.printf("%6d  %6d\n", t - millis(), t - told);
    told = millis();

    return;
}

void loopGUI(void *param)
{
    Button buttonOnOFF(200, 100, 100, 100);

    Button button1(0, 0, 160, 50);
    Button button2(0, 50, 160, 50);
    Button button3(0, 100, 160, 50);
    Button button4(160, 0, 160, 50);
    Button button5(160, 50, 160, 50);

    Button buttonMinus(0, 200, 107, 50);
    Button buttonPlus(214, 200, 106, 50);

    draw();

    while (true)
    {
        float inc = 0;

        M5.update();

        if (M5.BtnB.isPressed())
        {
            turn = 0;
            turn_frac0 = turn_frac;
            turnTotal_avg = turn_frac;
        }

        if (M5.BtnA.wasPressed() || (M5.BtnA.pressedFor(300)) || buttonMinus.wasPressed() || (buttonMinus.pressedFor(300)))
            inc = -1;
        if (M5.BtnA.pressedFor(1000) || buttonMinus.pressedFor(1000))
            inc = -5;

        if (M5.BtnC.wasPressed() || (M5.BtnC.pressedFor(300)) || buttonPlus.wasPressed() || (buttonPlus.pressedFor(300)))
            inc = +1;
        if (M5.BtnC.pressedFor(1000) || buttonPlus.pressedFor(1000))
            inc = +5;

        if (buttonOnOFF.wasPressed())
        {
            outputEnable = !outputEnable;

            if (outputEnable)
                pid1.SetMode(AUTOMATIC);
            else
                pid1.SetMode(MANUAL);

            draw(3);
        }

        if (button1.wasPressed())
        {
            selection = 1;
            draw(0);
        }
        if (button2.wasPressed())
        {
            selection = 2;
            draw(0);
        }
        if (button3.wasPressed())
        {
            selection = 3;
            draw(0);
        }
        if (button4.wasPressed())
        {
            selection = 4;
            draw(0);
        }
        if (button5.wasPressed())
        {
            selection = 5;
            draw(0);
        }

        if (button1.pressedFor(2000))
        {
            pid1.SetTunings(0, pid1.GetKi(), pid1.GetKd());
            paramValueChanged = true;
            draw(0);
        }
        if (button2.pressedFor(2000))
        {
            pid1.SetTunings(pid1.GetKp(), 0, pid1.GetKd());
            paramValueChanged = true;
            draw(0);
        }
        if (button3.pressedFor(2000))
        {
            pid1.SetTunings(pid1.GetKp(), pid1.GetKi(), 0);
            paramValueChanged = true;
            draw(0);
        }
        if (button4.pressedFor(2000))
        {
            pid1.SetOutputLimits(-100, 100);
            paramValueChanged = true;
            draw(0);
        }
        if (button5.pressedFor(2000))
        {
            xset = 0;
            paramValueChanged = true;
            draw(0);
        }

        if (inc != 0)
        {
            paramValueChanged = true;

            float gp = pid1.GetKp();
            float gi = pid1.GetKi();
            float gd = pid1.GetKd();

            float max = pid1.GetOutMax();

            switch (selection)
            {
            case 1:
                gp += inc * 0.1;
                break;
            case 2:
                gi += inc * 0.1;
                break;
            case 3:
                gd += inc * 0.1;
                break;
            case 4:
                max += inc;
                break;
            case 5:
                xset += inc * 0.1;
                break;
            }

            pid1.SetTunings(gp, gi, gd);
            pid1.SetOutputLimits(-max, max);

            draw(0);
        }

        drawCardran(120, 240 - 80, 80, xact, xset, outputGUI);

        delay(20);

        static unsigned long lastChanged = 0;
        if (paramValueChanged && (millis() - lastChanged) > 2000UL) // 2 second latency to avoid repeated write to the EEPROM
        {
            save_tuning(pid1);
            paramValueChanged = false;
            Serial.println("parameters saved!");
        }
    }
}