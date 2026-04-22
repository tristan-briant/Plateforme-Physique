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

    int color, pressedColor;
    String text;

    bool active = true;

public:
    Button(int x, int y, int w, int h) : x(x), y(y), w(w), h(h)
    {
    }

    Button(int x, int y, int w, int h, int col, int pressCol, String text)
        : x(x), y(y), w(w), h(h), color(col), pressedColor(pressCol), text(text)
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

            if (active && contain(t.x, t.y) && t.isPressed())
                return true;
        }

        return false;
    }

    bool isFlicking()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (active && contain(t.base_x, t.base_y) && t.isFlicking())
                return true;
        }
        return false;
    }

    // static (pourquoi static ???)
    int deltaFlickedH()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (active && t.isFlicking())
                return t.deltaX();
        }

        return 0;
    }

    bool wasPressed()
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (active && contain(t.base_x, t.base_y) && t.wasPressed())
                return true;
        }

        return false;
    }

    bool pressedFor(uint32_t ms)
    {
        for (std::size_t i = 0; i < M5.Touch.getCount(); ++i)
        {
            auto t = M5.Touch.getDetail(i);

            if (active && contain(t.base_x, t.base_y) && t.isPressed() && millis() - t.base_msec > ms)
                return true;
        }

        return false;
    }

    void draw()
    {
        active = true;
    }

    void hide()
    {
        active = false;
    }
};

#endif // BUTTON_H
