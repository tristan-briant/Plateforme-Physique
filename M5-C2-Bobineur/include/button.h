#if !defined(BUTTON_H)
#define BUTTON_H

#include <M5Unified.h>

uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return (r >> 3) << 11 | (g >> 2) << 5 | b >> 3; }
uint16_t color565(uint8_t grey) { return (grey & 0xF8) << 8 | (grey & 0xFC) << 3 | grey >> 3; }

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

    bool active = true;
    bool activated = false;

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
        if (isActive())
            return false;
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

    bool wasReleased()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.wasReleased())
                return true;
        }

        return false;
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

    bool wasClicked()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.wasClicked())
                return true;
        }

        return false;
    }

    bool pressedFor(uint32_t ms)
    {
        static bool triggered = true;

        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.isPressed() && millis() - t.base_msec > ms)
                return true;
        }

        return false;
    }

    bool pressedForOneTime(uint32_t ms)
    {
        static bool triggered = true;

        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.isPressed())
                if (millis() - t.base_msec < ms)
                    triggered = false;
                else if (!triggered)
                {
                    triggered = true;
                    return true;
                }
            return false;
        }

        return false;
    }

    bool releasedAfter(uint32_t ms)
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (contain(t.base_x, t.base_y) && t.wasReleased() && millis() - t.base_msec > ms)
                return true;
        }

        return false;
    }

    void setActive(bool val, bool redraw = false)
    {
        active = val;
        activated = false; // attend qu'il n'y ait plus de contact pour activer
        this->draw();
    }

    bool isActive()
    {
        if (activated)
            return true;

        if (active && M5.Touch.getCount() == 0)
        { // on active réellement s'il n'y a pas d'énènement
            activated = true;
            return true;
        }
        return false;
    }

    void draw()
    {
        if (active)
        {
            M5.Lcd.fillRoundRect(x, y, w, h, r, active ? fillColor : color565(50, 50, 50));
            M5.Lcd.drawRoundRect(x, y, w, h, r, lineColor);
            M5.Lcd.setTextDatum(CC_DATUM);
            M5.Lcd.setTextColor(active ? WHITE : color565(100, 100, 100));
            M5.Lcd.setFont(&FreeSansBold12pt7b);
            M5.Lcd.drawString(text, x + w / 2, y + h / 2);
        }
        else
            M5.Lcd.fillRect(x, y, w, h, BLACK);
    }
};

#endif // BUTTON_H
