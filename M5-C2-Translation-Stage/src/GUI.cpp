#include <M5Unified.h>
#include <Button.h>

extern double x_target, x_position;
extern double x;

M5Canvas img(&M5.Lcd);
M5Canvas mini_img(&M5.Lcd);

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3;
}

uint16_t color565(uint8_t grey)
{
    return (grey & 0xF8) << 8 | (grey & 0xFC) << 3 | grey >> 3;
}

float w10 = 220;
float r10 = 40;
float w1 = 80;
float r1 = 38;

void draw(int part = -1)
{
    int colorText = WHITE;
    int colorBackGround = BLACK;

    if (part < 0)
    {
        img.fillScreen(colorBackGround);

        img.setColor(100, 32, 32);
        img.fillRoundRect(10, 80, 300, 80, 40);
        img.setColor(100, 100, 100);
        img.drawRoundRect(10, 80, 300, 80, 40);

        img.setColor(140, 42, 42);
        img.fillRoundRect(80, 80, 160, 80, 40);
        img.setColor(100, 100, 100);
        img.drawRoundRect(80, 80, 160, 80, 40);

        img.setColor(255, 255, 255);
        /*img.drawString(">>", 320 / 2 + w10 / 2, 120);
        img.drawString("-10", 320 / 2 - w10 / 2, 120);
        img.drawString(">", 320 / 2 + w1 / 2, 120);
        img.drawString("-1", 320 / 2 - w1 / 2, 120);
        */

        int color = color565(255);
        // img.drawSmoothLine(10, 10, 50, 50, color);
        float x, y = 120, h = 15;
        x = 45;
        img.drawWideLine(x - 8, y, x + 8, y + h, 2, WHITE);
        img.drawWideLine(x - 8, y, x + 8, y - h, 2, WHITE);
        x = 55;
        img.drawWideLine(x - 8, y, x + 8, y + h, 2, WHITE);
        img.drawWideLine(x - 8, y, x + 8, y - h, 2, WHITE);
        x = 120;
        img.drawWideLine(x - 8, y, x + 8, y + h, 2, WHITE);
        img.drawWideLine(x - 8, y, x + 8, y - h, 2, WHITE);
        x = 200;
        img.drawWideLine(x + 8, y, x - 8, y + h, 2, WHITE);
        img.drawWideLine(x + 8, y, x - 8, y - h, 2, WHITE);
        x = 265;
        img.drawWideLine(x + 8, y, x - 8, y + h, 2, WHITE);
        img.drawWideLine(x + 8, y, x - 8, y - h, 2, WHITE);
        x = 275;
        img.drawWideLine(x + 8, y, x - 8, y + h, 2, WHITE);
        img.drawWideLine(x + 8, y, x - 8, y - h, 2, WHITE);
        img.pushSprite(0, 0);
    }

    if (part == 1)
    {
        mini_img.setTextColor(colorText, colorBackGround);
        mini_img.setTextDatum(MC_DATUM);
        mini_img.setFont(&FreeSans12pt7b);
        float y = 120;
        char buf[20];
        snprintf(buf, 20, "  X = %6.1f  ", x_position);
        mini_img.drawString(buf, 160, 10);

        mini_img.pushSprite(0, 55);
    }

    return;
}

void TaskGUI(void *pvParameters)
{
    Button buttonPlus10(240, 80, 80, 80);
    Button buttonPlus1(160, 80, 80, 80);
    Button buttonMinus1(80, 80, 80, 80);
    Button buttonMinus10(0, 80, 80, 80);
    Button buttonSlider(0, 0, 320, 80);

    img.createSprite(320, 240); // Create a 320x240 canvas
    mini_img.createSprite(320, 20);

    draw();
    delay(50);

    while (true)
    {

        M5.update();

        // if (buttonPlus10.wasPressed())
        if (buttonPlus10.wasReleased())
            x_target += 10;
        if (buttonPlus1.wasReleased())
            x_target += 1;
        if (buttonMinus10.wasReleased())
            x_target -= 10;
        if (buttonMinus1.wasReleased())
            x_target -= 1;

        // if (buttonSlider.isFlicking())
        //     x_target += (float)Button::deltaFlickedH()/3.0;

        // Serial.println(x_target);
        draw(1);
        delay(10);
    }
}
