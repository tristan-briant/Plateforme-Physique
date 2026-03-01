#include <M5Unified.h>
#include <Button.h>

extern float SPEED_MAX, ACC_MAX;
extern long delta;
extern long step, step_offset;

enum ModeRun
{
    MOTOR_ON,
    MODE_TURN,
    MOTOR_BREAK,
    MOTOR_RELEASED,
    PULSE // not used
};
extern ModeRun mode_run;

int selection = 0;
const int NSELECTION = 2;

int mainColor = DARKCYAN;
int colorText = WHITE;
int colorBackGround = BLACK;

// extern float freq, amp, offset, power;
extern double TotalTurn, speed, targetSpeed, Acceleration;
extern double period;
extern double TargetTurn, TurnToTarget, TotalTurn, lastTargetTurn;

bool display_turn = true;

// extern float offsetTarget, offsetCal;

M5Canvas img(&M5.Lcd);
M5Canvas img_small(&M5.Lcd);

/*uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3;
}*/

void draw(int part = -1)
{
    int ypos[] = {50, 100, 150, 200};
    int xpos;
    char str[20];

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
        int precision = 1; // fabs(targetSpeed) > 9.99 ? 1 : 2;
        img.drawFloat(targetSpeed, precision, 240, 50 + 2);

        img.pushSprite(0, 0);
    }
    if (part < 0 || part == 1) /// Draw the turn frame
    {
        img.fillRect(5, 0, 320 - 10, 50, selection == 1 ? mainColor : BLACK);
        img.drawRect(5, 0, 320 - 10, 50, mainColor);

        img.setTextColor(colorText);
        img.setTextDatum(BL_DATUM);
        img.setFont(&FreeSans18pt7b);
        img.drawString("Turn", 10, 50);
        // img.drawString("tr", 250, 50);
        img.setTextDatum(BR_DATUM);
        img.setFont(&FreeSansBold24pt7b);
        img.drawFloat(TargetTurn, 0, 240, 50 + 2);

        img.pushSprite(0, 50);
    }
    if (part < 0 || part == 3) /// Draw the On/Off frame
    {
        img_small.fillSprite(BLACK);
        if (mode_run == MODE_TURN)
        {
            img_small.fillRoundRect(5, 0, 90, 50, 15, color565(60));
            img_small.drawRoundRect(5, 0, 90, 50, 15, mainColor);
            img_small.setTextDatum(CC_DATUM);
            img_small.setTextColor(WHITE);
            img_small.setFont(&FreeSansBold12pt7b);
            // img_small.drawString(text, x + w / 2, y + h / 2);

            sprintf(str, "%.1f", TurnToTarget);
            img_small.drawString(str, 50, 25, &FreeSans12pt7b);
            img_small.pushSprite(10, 125);
        }
        // img.pushSprite(0, 50);
    }
    if (part < 0 || part == 4) /// Draw the speed/turn value frame
    {
        img.fillSprite(BLACK);
        img.setTextColor(colorText);
        // img.setFont(&FreeSans12pt7b);
        img.setTextDatum(BC_DATUM);

        img.setFont(&FreeSansBold18pt7b);
        img.fillRoundRect(320 / 6 - 50, 2, 100, 100, 15, color565(30));
        img.drawRoundRect(320 / 6 - 50, 2, 100, 100, 15, color565(100, 0, 0));
        img.drawString("-", 320 / 6, 38);

        // img.drawString("+", 320 /2, 38);

        img.fillRoundRect(320 * 5 / 6 - 50, 2, 100, 100, 15, color565(30));
        img.drawRoundRect(320 * 5 / 6 - 50, 2, 100, 100, 15, color565(100, 0, 0));
        img.drawString("+", 320 * 5 / 6, 38);

        img.fillRoundRect(320 / 2 - 50, 2, 100, 100, 15, color565(30));
        img.drawRoundRect(320 / 2 - 50, 2, 100, 100, 15, color565(100, 0, 0));
        img.setFont(&FreeSans12pt7b);
        sprintf(str, "%.1f", TotalTurn);

        img.drawString(str, 320 / 2, 28, &FreeSans12pt7b);
        img.drawString("reset", 320 / 2, 45, &FreeSans9pt7b);

        img.pushSprite(0, 200);
    }

    // Serial.printf("%6d  %6d\n", t - millis(), t - told);
    told = millis();

    return;
}

void TaskGUI(void *pvParameters)
{
    Button buttonSpeed(0, 0, 320, 50);
    Button buttonTurn(0, 50, 320, 50);
    // Button buttonOffsetSelec(0, 100, 320, 50);

    const int wb = 90, hb = 50;
    Button buttonNTurn(60 - wb / 2, 150 - hb / 2, wb, hb, 15, mainColor, color565(60), "N turn");
    Button buttonStop(160 - wb / 2, 150 - 1.8 * hb / 2, wb, 1.8 * hb, 15, mainColor, color565(200, 0, 0), "STOP");
    Button buttonInfinite(260 - wb / 2, 150 - hb / 2, wb, hb, 15, mainColor, color565(60), "Infinite");

    Button buttonMinus(0, 200, 107, 50);
    Button buttonPlus(214, 200, 106, 50);

    img.createSprite(320, 50);       // Create a 320x50 canvas
    img_small.createSprite(100, 50); // Create a 320x50 canvas

    draw();
    buttonStop.setActive(false);
    buttonInfinite.setActive(true);
    buttonNTurn.setActive(true);

    delay(50);

    while (true)
    {
        float inc = 0;

        M5.update();

        if (M5.BtnA.wasPressed() || (M5.BtnA.pressedFor(300)) || buttonMinus.wasPressed() || (buttonMinus.pressedFor(300)))
            inc = -1;
        if (M5.BtnA.pressedFor(1000) || buttonMinus.pressedFor(1000))
            inc = -10;
        /*if (M5.BtnA.pressedFor(3000) || buttonMinus.pressedFor(3000))
            inc = -100;*/
        if (M5.BtnC.wasPressed() || (M5.BtnC.pressedFor(300)) || buttonPlus.wasPressed() || (buttonPlus.pressedFor(300)))
            inc = +1;
        if (M5.BtnC.pressedFor(1000) || buttonPlus.pressedFor(1000))
            inc = +10;
        /*if (M5.BtnC.pressedFor(3000) || buttonPlus.pressedFor(3000))
            inc = +100;*/

        if (M5.BtnB.wasClicked())
            display_turn = !display_turn;

        if (M5.BtnB.pressedFor(1000))
            step = 0;

        if (buttonSpeed.isFlicking())
            inc = Button::deltaFlickedH();

        if (buttonTurn.isFlicking())
            inc = Button::deltaFlickedH() / 10.0;

        if (buttonSpeed.wasPressed())
        {
            selection = 0;
            draw();
        }

        if (buttonSpeed.pressedForOneTime(1000))
        {
            targetSpeed = -targetSpeed;
            draw();
        }

        if (buttonTurn.wasPressed())
        {
            selection = 1;
            draw();
        }
        if (buttonTurn.pressedFor(1000))
        {
            TargetTurn = 1;
            draw();
        }

        if (buttonNTurn.wasClicked())
        {
            mode_run = ModeRun::MODE_TURN;
            TurnToTarget = TargetTurn;
            step_offset = step;
            buttonNTurn.setActive(false);
            buttonInfinite.setActive(false);
            buttonStop.setActive(true);
        }

        if (buttonInfinite.wasClicked())
        {
            mode_run = ModeRun::MOTOR_ON;
            buttonInfinite.setActive(false);
            buttonNTurn.setActive(false);
            buttonStop.setActive(true);
        }

        if (mode_run == MOTOR_RELEASED && buttonStop.active == true)
        {
            buttonStop.setActive(false);
            buttonInfinite.setActive(true);
            buttonNTurn.setActive(true);
        }

        if (buttonStop.wasReleased())
        {
            mode_run = ModeRun::MOTOR_RELEASED;
            buttonStop.setActive(false);
            buttonInfinite.setActive(true);
            buttonNTurn.setActive(true);
        }

        if (inc != 0)
        {
            switch (selection)
            {
            case 0:
                /*if (fabs(targetSpeed + inc * 0.01) > 10 && abs(inc) == 1)
                    inc *= 10;*/
                if (inc > 0)
                    inc = 1;
                else
                    inc = -1;
                targetSpeed = constrain(targetSpeed + inc * 0.01, -SPEED_MAX, SPEED_MAX);
                break;
            case 1:
                TargetTurn = constrain(TargetTurn + inc, 1, 1000);
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

        if (c % 25 == 0)
        {
            draw(4); // refresh speed value
            draw(3);
        }

        c++;

        delay(1);
    }
}