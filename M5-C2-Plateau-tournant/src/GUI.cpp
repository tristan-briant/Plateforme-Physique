#include <M5Unified.h>
#include <Button.h>

extern float SPEED_MAX, ACC_MAX;
extern long delta;
extern long step;

enum ModeRun
{
    MOTOR_ON,
    SINUS,
    MOTOR_BREAK,
    MOTOR_RELEASED,
    PULSE // not used
};
extern ModeRun mode_run;

int selection = 0;
const int NSELECTION = 2;

int mainColor = ORANGE;
int colorText = WHITE;
int colorBackGround = BLACK;

// extern float freq, amp, offset, power;
extern double speed, targetSpeed, Acceleration; // turn per second
extern double period;

// extern float offsetTarget, offsetCal;

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

    if (part < 0 || part == 0) /// Draw the Speed frame
    {
        img.fillRect(5, 0, 320 - 10, 50, selection == 0 ? mainColor : BLACK);
        img.drawRect(5, 0, 320 - 10, 50, mainColor);

        img.setTextColor(colorText);
        img.setTextDatum(BL_DATUM);
        img.setFont(&FreeSans18pt7b);
        img.drawString("Speed", 10, 50);
        img.drawString("t/s", 250, 50);
        img.setTextDatum(BR_DATUM);
        img.setFont(&FreeSansBold24pt7b);
        int precision = fabs(targetSpeed) > 9.99 ? 1 : 2;
        img.drawFloat(targetSpeed, precision, 240, 50 + 2);

        img.pushSprite(0, 0);
    }
    if (part < 0 || part == 1) /// Draw the Acc frame
    {
        img.fillRect(5, 0, 320 - 10, 50, selection == 1 ? mainColor : BLACK);
        img.drawRect(5, 0, 320 - 10, 50, mainColor);

        img.setTextColor(colorText);
        img.setTextDatum(BL_DATUM);
        img.setFont(&FreeSans18pt7b);
        img.drawString("Acc", 10, 50);
        img.drawString("t/s", 250, 50);
        img.setFont(&FreeSans12pt7b);
        img.drawString("2", 250 + 38, 50 - 17);
        img.setTextDatum(BR_DATUM);
        img.setFont(&FreeSansBold24pt7b);
        img.drawFloat(Acceleration, 2, 240, 50 + 2);

        img.pushSprite(0, 50);
    }
    if (part < 0 || part == 3) /// Draw the On/Off frame
    {
        img.fillSprite(BLACK);
        img.setTextColor(colorText);
        img.setFont(&FreeSansBold12pt7b);
        img.setTextDatum(CC_DATUM);

        int x = 320 * 3 / 12;
        img.fillRoundRect(x - 50, 2, 100, 46, 15, color565(60));
        img.drawRoundRect(x - 50, 2, 100, 46, 15, mainColor);
        img.drawString(mode_run == MOTOR_ON ? "Start" : "Stop", x, 25);
        img.setTextDatum(CC_DATUM);
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

    // Serial.printf("%6d  %6d\n", t - millis(), t - told);
    told = millis();

    return;
}

void TaskGUI(void *pvParameters)
{
    Button buttonFreqSelec(0, 0, 320, 50);
    Button buttonAmpSelec(0, 50, 320, 50);
    Button buttonOffsetSelec(0, 100, 320, 50);

    Button buttonOn(80 - 50, 125, 100, 50, 15, mainColor, color565(60), "Start");
    Button buttonFree(240 - 50, 125, 100, 50, 15, mainColor, color565(60), "free");

    Button buttonMinus(0, 200, 107, 50);
    Button buttonPlus(214, 200, 106, 50);

    img.createSprite(320, 50); // Create a 320x240 canvas

    draw();
    buttonFree.draw();
    buttonOn.draw();
    delay(50);

    while (true)
    {
        float inc = 0;

        M5.update();

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

        if (buttonOn.wasPressed())
        {
            if (mode_run == ModeRun::MOTOR_ON)
            {
                mode_run = ModeRun::MOTOR_BREAK;
                buttonOn.text = "Start";
                buttonOn.draw();
            }
            else
            {
                mode_run = ModeRun::MOTOR_ON;
                buttonOn.text = "Stop";
                buttonOn.draw();
            }
            draw(3);
        }

        if (buttonFree.isPressed())
        {
            mode_run = ModeRun::MOTOR_RELEASED;
            buttonOn.text = "Start";
            buttonOn.draw();
        }

        if (inc != 0)
        {
            switch (selection)
            {
            case 0:
                if (fabs(targetSpeed + inc * 0.01) > 10 && abs(inc) == 1)
                    inc *= 10;
                targetSpeed = constrain(targetSpeed + inc * 0.01, -SPEED_MAX, SPEED_MAX);
                break;
            case 1:
                Acceleration = constrain(Acceleration + inc * 0.01, 0, ACC_MAX);
                break;
            }
            draw(selection);
        }

        static int c = 0;
        static float speed_old = -1;
        if (c % 50 == 0)
        {
            if (speed != speed_old)
            {
                Serial2.printf("v=%3.2f\n", speed);
                speed_old = speed;
            }
            else
            {
                if (c % 1000 == 0)
                    Serial2.printf("u=tr/s\nv=%3.2f\n", speed);
            }
            Serial2.flush();
        }

        c++;

        delay(1);

        /*static int count = 0;
        count++;
        if (count % 100 == 0)
        {
            Serial.print(step);
            Serial.print("  ");
            Serial.println(period);
        }*/
    }
}
