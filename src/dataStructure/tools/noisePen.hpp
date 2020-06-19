#if !defined(NOISE_PEN_H)
#define NOISE_PEN_H

#include <random>

#include "Tool.hpp"
#include "../layer.hpp"
#include "../../color/color.h"

class NoisePen : public Tool
{
public:
    NoisePen()
    {
        setProperty("R", 0);
        setProperty("G", 0);
        setProperty("b", 0);
        setProperty("a", 255);
        setProperty("SIZE", 1);
        e.seed(wxGetLocalTime());
        tool_type = ToolType::PEN;
    }
    virtual void moveTo(int x, int y)
    {
        this->x = x;
        this->y = y;
        if (is_down && current_layer != NULL)
        {
            int size = getProperty("SIZE");
            if (size == -1)
            {
                size = 1;
            }
            x -= size / 2;
            y -= size / 2;

            for (int h = 0; h < size; h++)
            {
                for (int w = 0; w < size; w++)
                {
                    int gray = u(e);
                    current_layer->paint(x + w, y + h, RGBColor(gray, gray, gray, getProperty("A")));
                }
            }
        }
    }
    virtual void penDown()
    {
        is_down = true;
        moveTo(x, y);
    }
    virtual void penUp()
    {
        if (is_down && current_layer != NULL)
        {
            current_layer->stroke();
        }
    }
    virtual void setFunctionalKeys(bool shift, bool ctrl)
    {
    }

protected:
    int x, y;
    default_random_engine e;
    uniform_int_distribution<int> u = uniform_int_distribution<int>(0, 255);
};


#endif // NOISE_PEN_H
