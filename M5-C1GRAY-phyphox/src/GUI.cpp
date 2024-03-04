#include <M5Stack.h>
//#include "bt-util.h"
#include <pref-util.h>

extern float accData[4];
extern float gyrData[4];
extern float magData[4];

extern float accRaw[4];
extern float gyrRaw[4];
extern float magRaw[4];

extern float accBias[4];
extern float gyrBias[4];
extern float magBias[4];
extern float accScale[4];

extern float angleX, angleY, angleZ;

extern int batteryLevel;
extern bool isCharging;
extern bool fullCharge;

TFT_eSprite img = TFT_eSprite(&M5.Lcd);

void magAverage(int step);
void DrawBatteryStatus();
void DrawBatteryStatusGraphic();

void Screen_begin()
{
    img.setColorDepth(8);
    img.setTextColor(TFT_WHITE);
    img.createSprite(320, 240);
    img.setBitmapColor(TFT_WHITE, 0);
}

void redraw()
{
    static int page = 0; //start page = 0
    static int step = 0;
    static int currentPage = -1;

    if (currentPage == -1)
        Screen_begin();

    switch (page)
    {
    case 0: // main page
    {
        img.fillSprite(BLACK);

        DrawBatteryStatusGraphic();
        img.setTextColor(WHITE);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        //img.setCursor(160, 24);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString(getDeviceName(), 160, 24);
        img.setTextFont(4);
        img.setTextColor(WHITE);
        img.drawString("Options", 60, 210);
        img.drawString("Off", 160, 210);
        img.drawString("Calib.", 320 - 60, 210);

        img.setTextDatum(BR_DATUM);
        img.setTextFont(4);
        img.setTextColor(YELLOW);

        const int x[] = {80, 160, 240};

        char buf[50];
        img.setTextColor(YELLOW, BLACK);

        for (int i = 2; i >= 0; i--)
        {
            sprintf(buf, "%8d", (int)(1000 * accData[i + 1]));
            img.drawString(buf, x[i], 48 * 2);
        }
        img.drawString("mg", 300, 48 * 2);

        for (int i = 2; i >= 0; i--)
        {
            sprintf(buf, "%8d", (int)(1 * gyrData[i + 1]));
            img.drawString(buf, x[i], 48 * 3);
        }
        img.drawString("o/s", 300, 48 * 3);

        for (int i = 2; i >= 0; i--)
        {
            sprintf(buf, "%8d", (int)(1 * magData[i + 1]));
            img.drawString(buf, x[i], 48 * 4);
        }
        img.drawString("mG", 300, 48 * 4);

        img.pushSprite(0, 0);

        if (M5.BtnA.wasReleased()) // if button A puched go to Option page
            page = 1;

        if (M5.BtnB.wasReleased()) // if button A puched go to Shut down page
            page = 50;

        if (M5.BtnC.wasReleased()) // if button C puched go to Cal page
            page = 100;
    }
    break;

    case 1:
    { //page option NAME
        img.fillSprite(BLACK);
        img.setTextColor(WHITE, BLACK);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString("NAME", 160, 24);
        img.setTextFont(2);
        img.setTextColor(WHITE, BLACK);
        img.drawString("(restart required)", 160, 55);
        img.setTextFont(4);
        img.drawString("Next", 60, 210);
        img.drawString("-", 160, 210);
        img.drawString("+", 320 - 60, 210);
        img.setTextFont(4);
        img.setTextDatum(MC_DATUM);
        img.setTextColor(GREENYELLOW, BLACK);
        img.drawString(getDeviceName(), 160, 120);
        img.pushSprite(0, 0);

        if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(300))
            increment_name(-1);
        if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(300))
            increment_name(1);

        if (M5.BtnA.wasReleased())
            page++;
    }
    break;

    case 2:
    { // LCD Brigthness
        img.fillSprite(BLACK);
        img.setTextColor(WHITE, BLACK);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString("Screen Brightness ", 160, 24);
        img.setTextFont(2);
        img.setTextColor(WHITE, BLACK);
        img.drawString("low values can save up to 50% lifetime", 160, 65);
        img.setTextFont(4);
        img.drawString("Next", 60, 210);
        img.drawString("-", 160, 210);
        img.drawString("+", 320 - 60, 210);
        img.setTextFont(4);
        img.setTextDatum(MC_DATUM);
        img.setTextColor(GREENYELLOW, BLACK);
        img.setCursor(160, 120);
        extern int brightness;
        img.printf("%d %%", (int)(brightness / 2.55));
        img.pushSprite(0, 0);

        if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(300))
            M5.Lcd.setBrightness(increment_brightness(-3));
        if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(300))
            M5.Lcd.setBrightness(increment_brightness(+3));

        if (M5.BtnA.wasReleased())
            page++;
    }
    break;

    case 50:
    { //page Shut down
        img.fillSprite(BLACK);
        img.setTextColor(RED);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString("<--- Double click", 160, 24);
        img.drawString("    red button", 160, 60);
        img.setTextFont(4);
        img.setTextColor(WHITE);
        img.drawString("Back", 60, 210);
        img.pushSprite(0, 0);

        if (M5.BtnA.wasReleased())
            page = 0;
    }
    break;

    case 100:
    { // Acc calibration
        const int N_AVG = 100;
        float bx = 0, by = 0, bz = 0, accgx = 0, accgy = 0, accgz = 0;

        img.fillSprite(BLACK);
        img.setTextColor(WHITE, BLACK);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString("CALIBRATION", 160, 24);
        img.drawString("Accelerometer", 160, 60);

        img.setTextFont(4);
        img.setTextColor(WHITE, BLACK);
        img.drawString("Next", 320 - 60, 210);

        switch (step)
        {
        case 0: // Ask for positioning Z
            img.setTextColor(GREENYELLOW, BLACK);
            img.drawString("Put the sensor on a table", 160, 110);
            img.drawString("  SCREEN UP  ", 160, 140);
            img.drawString("push start, wait 1 second", 160, 170);
            img.setTextColor(WHITE);
            img.drawString("Start", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                step++;
            break;

        case 1: //start cal Z + gyro
            img.setTextColor(GREENYELLOW, BLACK);
            img.drawString("CALIBRATING Z AXIS...", 160, 140);
            //img.drawString("AND GYRO...", 160, 170);
            img.pushSprite(0, 0);
            delay(500);

            for (int i = 0; i < N_AVG; i++)
            {
                bx += accRaw[1];
                by += accRaw[2];
                accgz += accRaw[3];

                /*bgx += gyrRaw[1];
                bgy += gyrRaw[2];
                bgz += gyrRaw[3];*/
                delay(2);
            }

            bx = bx / N_AVG;
            by = by / N_AVG;
            accgz = accgz / N_AVG;

            if (accgz > 1.5 || accgz < 0.5) // FAIL
            {
                step = -1;
                break;
            }

            /*gyrBias[1] = bgx / N_AVG;
            gyrBias[2] = bgy / N_AVG;
            gyrBias[3] = bgz / N_AVG;*/
            step++;
            break;

        case 2: // Ask for positioning Y
            img.setTextColor(PINK, BLACK);
            img.drawString("Put the sensor on a table", 160, 110);
            img.drawString("    STANDING UP    ", 160, 140);
            img.drawString("push start, wait 1 second", 160, 170);
            img.setTextColor(WHITE);
            img.drawString("Start", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                step++;
            break;

        case 3: //start cal Y
            img.setTextColor(PINK, BLACK);
            img.drawString("CALIBRATING Y AXIS...", 160, 140);
            img.pushSprite(0, 0);
            delay(500);

            for (int i = 0; i < N_AVG; i++)
            {
                bz += accRaw[3];
                accgy += accRaw[2];
                delay(2);
            }

            bz = bz / N_AVG;
            accgy = accgy / N_AVG;

            if (accgy > 1.5 || accgy < 0.5) // FAIL
            {
                step = -1;
                break;
            }

            step++;
            break;

        case 4: // Ask for positioning X
            img.setTextColor(CYAN, BLACK);
            img.drawString("Put the sensor on a table", 160, 110);
            img.drawString("    ON THE SIDE    ", 160, 140);
            img.drawString("push start, wait 1 second", 160, 170);
            img.setTextColor(WHITE);
            img.drawString("Start", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                step++;
            break;

        case 5:
            img.setTextColor(CYAN, BLACK);
            img.drawString("CALIBRATING X AXIS...", 160, 140);
            img.pushSprite(0, 0);
            delay(500);

            for (int i = 0; i < N_AVG; i++)
            {
                accgx += accRaw[1];
                delay(2);
            }

            accgx = accgx / N_AVG;

            if (abs(accgx) > 1.5 || abs(accgx) < 0.5) // FAIL
            {
                step = -1;
                break;
            }

            accBias[1] = bx;
            accBias[2] = by;
            accBias[3] = bz;

            if (accgx > 0)
                accScale[1] = 1 / (accgx - bx);
            else
                accScale[1] = 1 / (-accgx + bx);

            accScale[2] = 1 / (accgy - by);
            accScale[3] = 1 / (accgz - bz);

            //save_cal("gyrometer", gyrBias);
            save_cal("accel_bias", accBias);
            save_cal("accel_scale", accScale);

            step++;
            break;

        case -1: //FAILLED
            img.setTextColor(RED, BLACK);
            img.drawString("FAILED", 160, 140);
            img.drawString("Not in a good position", 160, 170);
            img.setTextColor(WHITE);
            img.drawString("Retry", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                step = 0;
            break;

        default:
            img.setTextColor(WHITE, BLACK);
            img.drawString("Done !", 160, 140);
            img.pushSprite(0, 0);
        }

        if (M5.BtnC.wasReleased())
        {
            page++;
            step = 0;
        }
    }
    break;

    case 101:
    { // GYRO calib.

        const int N_AVG = 100;
        float bgx = 0, bgy = 0, bgz = 0;

        img.fillSprite(BLACK);
        img.setTextColor(WHITE, BLACK);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString("CALIBRATION", 160, 24);
        img.drawString("Gyrometer", 160, 60);

        img.setTextFont(4);
        img.setTextColor(WHITE, BLACK);
        img.drawString("Next", 320 - 60, 210);

        switch (step)
        {
        case 0: // Ask for positioning Z
            img.setTextColor(GREENYELLOW, BLACK);
            img.drawString("Put the sensor on a table", 160, 110);
            img.drawString("  SCREEN UP  ", 160, 140);
            img.drawString("push start, wait 1 second", 160, 170);
            img.setTextColor(WHITE);
            img.drawString("Start", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                step++;
            break;

        case 1: //start cal Z + gyro

            img.setTextColor(GREENYELLOW, BLACK);
            img.drawString("CALIBRATING", 160, 140);
            img.drawString("GYRO BIAS...", 160, 170);
            img.pushSprite(0, 0);
            delay(500);

            for (int i = 0; i < N_AVG; i++)
            {
                bgx += gyrRaw[1];
                bgy += gyrRaw[2];
                bgz += gyrRaw[3];
                delay(2);
            }

            gyrBias[1] = bgx / N_AVG;
            gyrBias[2] = bgy / N_AVG;
            gyrBias[3] = bgz / N_AVG;

            save_cal("gyrometer", gyrBias);
            angleZ = 0;
            step++;
            break;

        case 2: // Ask for positioning Y

            img.setTextColor(PINK, BLACK);
            img.setTextDatum(TL_DATUM);
            img.drawString("Test Gyro :", 5, 140);
            img.setCursor(150, 110);
            img.printf("ax=%7.2f", angleX);
            img.setCursor(150, 140);
            img.printf("ay=%7.2f", angleY);
            img.setCursor(150, 170);
            img.printf("az=%7.2f", angleZ);
            img.setTextColor(WHITE);
            img.setTextDatum(TC_DATUM);
            img.drawString("Reset", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                angleX = angleY = angleZ = 0;
            break;
            /*
        case 3: //start cal Y
            img.setTextColor(PINK, BLACK);
            img.drawString("CALIBRATING Y AXIS...", 160, 140);
            img.pushSprite(0, 0);
            delay(500);

            for (int i = 0; i < N_AVG; i++)
            {
                bz += accRaw[3];
                accgy += accRaw[2];
                delay(2);
            }

            bz = bz / N_AVG;
            accgy = accgy / N_AVG;

            if (accgy > 1.5 || accgy < 0.5) // FAIL
            {
                step = -1;
                break;
            }

            step++;
            break;

        case 4: // Ask for positioning X
            img.setTextColor(CYAN, BLACK);
            img.drawString("Put the sensor on a table", 160, 110);
            img.drawString("    ON THE SIDE    ", 160, 140);
            img.drawString("push start, wait 1 second", 160, 170);
            img.setTextColor(WHITE);
            img.drawString("Start", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                step++;
            break;

        case 5:
            img.setTextColor(CYAN, BLACK);
            img.drawString("CALIBRATING X AXIS...", 160, 140);
            img.pushSprite(0, 0);
            delay(500);

            for (int i = 0; i < N_AVG; i++)
            {
                accgx += accRaw[1];
                delay(2);
            }

            accgx = accgx / N_AVG;

            if (abs(accgx) > 1.5 || abs(accgx) < 0.5) // FAIL
            {
                step = -1;
                break;
            }

            accBias[1] = bx;
            accBias[2] = by;
            accBias[3] = bz;

            if (accgx > 0)
                accScale[1] = 1 / (accgx - bx);
            else
                accScale[1] = 1 / (-accgx + bx);

            accScale[2] = 1 / (accgy - by);
            accScale[3] = 1 / (accgz - bz);

            save_cal("gyrometer", gyrBias);
            save_cal("accel_bias", accBias);
            save_cal("accel_scale", accScale);

            step++;
            break;

        case -1: //FAILLED
            img.setTextColor(RED, BLACK);
            img.drawString("FAILED", 160, 140);
            img.drawString("Not in a good position", 160, 170);
            img.setTextColor(WHITE);
            img.drawString("Retry", 160, 210);
            img.pushSprite(0, 0);
            if (M5.BtnB.wasReleased())
                step = 0;
            break;
*/
        default:
            img.setTextColor(WHITE, BLACK);
            img.drawString("Done !", 160, 140);
            img.pushSprite(0, 0);
        }

        if (M5.BtnC.wasReleased())
        {
            page++;
            step = 0;
        }
    }
    break;

    case 102:
    { //calibration magnetometer
        img.fillSprite(0);
        img.setTextColor(WHITE, BLACK);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString("CALIBRATION", 160, 24);
        img.drawString("Magnetometer", 160, 60);
        img.setTextFont(4);
        img.setTextColor(WHITE, BLACK);
        img.drawString("Next", 320 - 60, 210);

        switch (step)
        {
        case 0:
            img.drawString("start cal", 160, 210);
            img.setTextColor(GREEN, BLACK);
            img.drawString("Press start to calibrate", 160, 140);
            if (M5.BtnB.wasReleased())
                step++;
            break;

        case 1:
            magAverage(0);
            step++;
        case 2:
            img.drawString("finish cal", 160, 210);
            img.setTextColor(GREEN, BLACK);
            img.drawString("Turn the sensor in your hand", 160, 110);
            img.drawString("in any direction", 160, 140);
            img.drawString("then press finish", 160, 170);
            if (M5.BtnB.wasReleased())
                step++;
            break;

        case 3:
            magAverage(2);
            magAverage(3);
            step++;
        case 4:
            img.setTextColor(WHITE, BLACK);
            img.drawString("Done !", 160, 140);
            break;
        }

        img.pushSprite(0, 0);

        if (M5.BtnC.wasReleased())
            page++;
    }
    break;

    case 103:
    { //Delete Calibration data
        img.fillSprite(0);
        img.setTextColor(WHITE, BLACK);
        img.setTextSize(1);
        img.setTextDatum(TC_DATUM);
        img.setFreeFont(&FreeSansBold18pt7b);
        img.drawString("Calibration data", 160, 24);
        //img.drawString("saved data", 160, 60);
        img.setTextFont(4);
        img.setTextColor(WHITE, BLACK);
        img.drawString("Next", 320 - 60, 210);
        img.drawString("Reset", 160, 210);
        //img.drawString("name=0", 320 - 60, 210);

        img.setTextDatum(BR_DATUM);
        img.setTextColor(CYAN, BLACK);
        char buf[80];
        const int x[] = {80, 160, 240};

        for (int i = 2; i >= 0; i--)
        {
            sprintf(buf, "%8d", (int)(1000 * accBias[i + 1]));
            img.drawString(buf, x[i], 48 * 2);
        }
        img.drawString("mg", 300, 48 * 2);

        for (int i = 2; i >= 0; i--)
        {
            sprintf(buf, "%8d", (int)(1 * gyrBias[i + 1]));
            img.drawString(buf, x[i], 48 * 3);
        }
        img.drawString("o/s", 300, 48 * 3);

        for (int i = 2; i >= 0; i--)
        {
            sprintf(buf, "%8d", (int)(1 * magBias[i + 1]));
            img.drawString(buf, x[i], 48 * 4);
        }
        img.drawString("mG", 300, 48 * 4);

        img.pushSprite(0, 0);

        if (M5.BtnB.wasReleased())
            clear_all_data();

        if (M5.BtnC.wasReleased())
            page++;
    }
    break;

    default:
        page = 0; // end of cycle page
    }
}

void DrawBatteryStatus()
{
    static char buf[20];

    if (batteryLevel != -1)
    {
        if (fullCharge)
            sprintf(buf, "Batt. Full");
        else if (isCharging)
            sprintf(buf, "Charging...");
        else
            sprintf(buf, "Batt. %d%%", batteryLevel);

        if (batteryLevel <= 25)
            img.setTextColor(RED);
        else if ((batteryLevel <= 50))
            img.setTextColor(ORANGE);
        else
            img.setTextColor(CYAN);

        //img.setTextFont(2);
        img.setFreeFont(&FreeSans9pt7b);
        img.setTextDatum(TR_DATUM);

        img.drawString(buf, 315, 0);
    }
    img.setTextColor(CYAN);
    img.setFreeFont(&FreeSans9pt7b);
    img.setTextDatum(TL_DATUM);
    long s = millis() / 1000;
    int h = s / 3600;
    s = s % 3600;
    int m = s / 60;
    s = s % 60;
    sprintf(buf, "%2d:%02d:%02d", h, m, (int)s);
    img.drawString(buf, 5, 0);
}

void DrawBatteryStatusGraphic()
{
    static char buf[20];

    const int bar_w = 7;
    const int bar_h = 12;
    const int x = 320 - 45, y = 2, h = bar_h + 6, w = bar_w * 4 + 9;

    uint32_t color;
    if (batteryLevel != -1)
    {
        if (isCharging)
            color = WHITE;
        else if (batteryLevel <= 25)
            color = RED;
        else if (batteryLevel <= 50)
            color = ORANGE;
        else
            color = CYAN;

        img.drawRect(x, y, w, h, color);
        img.drawRect(x + 1, y + 1, w - 2, h - 2, color);
        img.fillRect(x + w, y + 4, 2, h - 8, color);

        int nbar = batteryLevel / 25;

        if (isCharging && !fullCharge && (millis() / 1000) % 2)
            nbar--;

        for (int i = 0; i < nbar; i++)
        {
            img.fillRect(x + 3 + i * (bar_w + 1), y + 3, bar_w, bar_h, color);
        }
    }

    img.setTextColor(CYAN);
    img.setFreeFont(&FreeSans9pt7b);
    img.setTextDatum(TL_DATUM);
    long s = millis() / 1000;
    int hh = s / 3600;
    s = s % 3600;
    int m = s / 60;
    s = s % 60;
    sprintf(buf, "%2d:%02d:%02d", hh, m, (int)s);
    img.drawString(buf, 5, 0);
}