#if !defined(ERASER_H)
#define ERASER_H

#include "Tool.hpp"
#include "../layer.hpp"
#include "../../color/color.h"

class Eraser : public Tool
{
public:
    Eraser()
    {
        setProperty("R", 0);
        setProperty("G", 0);
        setProperty("b", 0);
        setProperty("a", 255);
        setProperty("SIZE", 1);
        tool_type = ToolType::PEN;
        
        icon = _T("eraser.png");
    }
    virtual void moveTo(int x, int y)
    {
        this->x = x;
        this->y = y;
        if (is_down && current_layer != NULL)
        {
            int size = getProperty("SIZE");
            if (size <= 0)
            {
                size = 1;
            }
            x -= size / 2;
            y -= size / 2;

            for (int h = 0; h < size; h++)
            {
                for (int w = 0; w < size; w++)
                {
                    current_layer->paint(x + w, y + h, RGBColor(0, 0, 0, 0));
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
};


#endif // ERASER_H
