#include <M5Stack.h>
extern int batteryLevel;
extern bool isCharging;
extern bool fullCharge;
extern bool recording;
extern bool SDCardPresent;

extern long CO2;

void DrawHeader();
void DrawBarGraph();
char *DrawTime(int format);
void IncrementDateTime(int selection, int increment);
void startrec();
void stoptrec();
bool test_SDCard();

TFT_eSprite img = TFT_eSprite(&M5.Lcd);

void Screen_begin()
{
    img.setColorDepth(8);
    img.setTextColor(TFT_WHITE);
    img.createSprite(320, 240);
    img.setBitmapColor(TFT_WHITE, 0);
}

void redraw()
{
    static long tinit = millis();
    static int page = 0; //start page = 0
    static int step = 0;

    img.fillSprite(BLACK);
    //img.drawString(DrawTime(), 5, 0);
    DrawHeader();

    img.setTextSize(1);
    img.setTextColor(WHITE);
    img.setTextDatum(TC_DATUM);

    char buf[50];

    switch (page)
    {
    case 0: // main page
    {
        int t = millis();
        if (t - tinit > 3000)
        {
            img.setFreeFont(&FreeSansBold12pt7b);
            img.drawString("CO2 concentration", 160, 60);
            img.setFreeFont(&FreeSansBold24pt7b);

            sprintf(buf, "%d ppm", (int)CO2);
            img.drawString(buf, 160, 100);

            DrawBarGraph();
        }
        else
        {
            img.setFreeFont(&FreeSansBold12pt7b);
            sprintf(buf, "Initialization... %d", 3 - (int)(t - tinit) / 1000);
            img.drawString(buf, 160, 60);
        }

        ////// banière basse
        img.setTextFont(4);
        img.setTextColor(WHITE);
        img.drawString("Options", 60, 210);
        img.drawString("Off", 160, 210);
        if (SDCardPresent && !recording)
        {
            img.drawString("Star Rec", 260, 210);
            if (M5.BtnC.wasReleased()) // if button A puched go to Shut down page
                startrec();
        }
        else if (recording)
        {
            img.setTextColor(RED);
            img.drawString("Recording", 260, 210);
            if (M5.BtnC.wasReleased()) // if button A puched go to Shut down page
                stoptrec();
        }
        else
        //if (!SDCardPresent)
        {
            img.setTextColor(DARKGREY);
            img.drawString("no SD", 260, 210);
            if (M5.BtnC.wasReleased()) // if button A puched go to Shut down page
                test_SDCard();
        }

        if (M5.BtnA.wasReleased())
        { // if button A puched go to Option page
            page = 1;
            step = 0;
        }

        if (M5.BtnB.wasReleased()) // if button A puched go to Shut down page
            page = 50;
    }
    break;
    case 1: // option page
    {
        img.setTextFont(4);
        img.setTextColor(WHITE);
        img.drawString("-", 60, 210);
        img.drawString("Select", 160, 210);
        img.drawString("+", 260, 210);

        img.drawString("ADJUST:", 160, 50);
        img.setTextColor(step == 0 ? ORANGE : WHITE);
        img.drawString("year", 160, 80);
        img.setTextColor(step == 1 ? ORANGE : WHITE);
        img.drawString("month", 160, 100);
        img.setTextColor(step == 2 ? ORANGE : WHITE);
        img.drawString("day", 160, 120);
        img.setTextColor(step == 3 ? ORANGE : WHITE);
        img.drawString("hour", 160, 140);
        img.setTextColor(step == 4 ? ORANGE : WHITE);
        img.drawString("minutes", 160, 160);

        if (M5.BtnA.wasReleased())
            IncrementDateTime(step, -1);

        if (M5.BtnC.wasReleased()) // if button A puched go to Option page
            IncrementDateTime(step, +1);

        if (M5.BtnB.wasReleased()) // if button A puched go to Shut down page
        {
            step++;
            if (step == 5)
                page = 0;
        }
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
    }

    if (page == 2)
        page = 0;
    img.pushSprite(0, 0);
}

void DrawHeader()
{
    //static char buf[20];

    const int bar_w = 7;
    const int bar_h = 12;
    const int x = 320 - 45, y = 2, h = bar_h + 6, w = bar_w * 4 + 9;

    Wire.begin();
    M5.Power.begin();
    batteryLevel = M5.Power.getBatteryLevel();
    isCharging = M5.Power.isCharging();
    fullCharge = M5.Power.isChargeFull();

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
    img.drawString(DrawTime(0), 5, 0);
    /*long s = millis() / 1000;
    int hh = s / 3600;
    s = s % 3600;
    int m = s / 60;
    s = s % 60;
    sprintf(buf, "%2d:%02d:%02d", hh, m, (int)s);
    img.drawString(buf, 5, 0);*/
}

void DrawBarGraph()
{

    img.drawRect(0, 160, 320, 30, WHITE);

    uint32_t color;

    if (CO2 > 1200)
        color = RED;
    else if (CO2 > 800)
        color = ORANGE;
    else
        color = GREEN;

    int min = 400, max = 2400;
    float x = constrain((float)(CO2 - min) / (max - min), 0.01, 1);
    int w = 318 * x;

    img.fillRect(1, 161, 318, 28, DARKGREY);
    img.fillRect(1, 161, w, 28, color);
}
