#if !defined(BUTTON_H)
#define BUTTON_H

#include <M5Unified.h>

// Home made mini lib to emulate button from M5core2 not present in M5Unified

class Button
{
public:
    int x;
    int y;
    int w;
    int h;

    int r = 0; // radiur of round box
    int lineColor, fillColor;
    String text;

public:
    Button(int x, int y, int w, int h) : x(x), y(y), w(w), h(h)
    {
    }

    Button(int x, int y, int w, int h, int r, int lineColor, int fillColor, String text)
        : x(x), y(y), w(w), h(h), r(r), lineColor(lineColor), fillColor(fillColor), text(text)
    {
    }

    bool contain(int x, int y)
    {
        return this->x <= x && x < (this->x + this->w) && this->y <= y && y < (this->y + this->h);
    }

    bool isPressed()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.x, t.y) && t.isPressed())
                return true;
        }

        return false;
    }

    bool isFlicking()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.isFlicking())
                return true;
        }
        return false;
    }

    static int deltaFlickedH()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (t.isFlicking())
                return t.deltaX();
        }

        return 0;
    }

    bool wasPressed()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.wasPressed())
                return true;
        }

        return false;
    }

    bool pressedFor(uint32_t ms)
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.isPressed() && millis() - t.base_msec > ms)
                return true;
        }

        return false;
    }

    void draw()
    {
        M5.Lcd.fillRoundRect(x, y, w, h, r, fillColor);
        M5.Lcd.drawRoundRect(x, y, w, h, r, lineColor);
        M5.Lcd.setTextDatum(CC_DATUM);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setFont(&FreeSansBold12pt7b);
        M5.Lcd.drawString(text, x + w / 2, y + h / 2);
    }
};

#endif // BUTTON_H
