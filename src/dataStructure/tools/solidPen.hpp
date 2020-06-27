#if !defined(SOLID_PEN)
#define SOLID_PEN

#include "Tool.hpp"
#include "../layer.hpp"
#include "../../color/color.h"

/**
 * simple pen
 * R, G, B, A, SIZE
 */
class SolidPen : public Tool
{
public:
    SolidPen()
    {
        setProperty("R", 0);
        setProperty("G", 0);
        setProperty("b", 0);
        setProperty("a", 255);
        setProperty("SIZE", 1);
        tool_type = ToolType::COLORPEN;
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
                    current_layer->paint(x + w, y + h, RGBColor(getProperty("R"), getProperty("g"), getProperty("B"), getProperty("A")));
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
        is_down = false;
    }
    virtual void setFunctionalKeys(bool shift, bool ctrl)
    {
    }

protected:
    int x, y;
};

#endif // SOLID_PEN
