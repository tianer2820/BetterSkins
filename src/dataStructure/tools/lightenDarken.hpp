#if !defined(LIGHTEN_DARKEN)
#define LIGHTEN_DARKEN

#include "Tool.hpp"
#include "../layer.hpp"
#include "../../color/color.h"

class LightenDarkenTool : public Tool
{
public:
    LightenDarkenTool()
    {
        setProperty("R", 0);
        setProperty("G", 0);
        setProperty("b", 0);
        setProperty("a", 255);
        setProperty("SIZE", 1);
        setProperty("LIGHTNESS", 10);
        tool_type = ToolType::PEN;
    }
    virtual void moveTo(int x, int y)
    {
        if(this->x == x && this->y == y && !just_clicked){
            // no move
            return;
        }
        just_clicked = false;
        
        this->x = x;
        this->y = y;
        if (is_down && current_layer != NULL)
        {
            int size = getProperty("SIZE");
            int lightness = getProperty("LIGHTNESS");
            int incremental = getProperty("INCREMENTAL");
            if (size == -1)
            {
                size = 1;
            }
            x -= size / 2;
            y -= size / 2;

            wxImage* img;
            wxImage render;
            if(incremental == 1){
                render = current_layer->renderRaw();
                img = &render;
            } else{
                img = current_layer->getImage();
            }
            for (int h = 0; h < size; h++)
            {
                for (int w = 0; w < size; w++)
                {
                    if(w+x<0 || w+x>img->GetWidth() || h+y<0 || h+y > img->GetHeight()){
                        continue;
                    }
                    int rgb[3] = {img->GetRed(w + x, h + y), img->GetGreen(w + x, h + y), img->GetBlue(w + x, h + y)};
                    int hsv[3];
                    Color::RGB2HSV(rgb, hsv);
                    hsv[2] = min(255, max(0, hsv[2] + lightness));

                    HSVColor color;
                    color.setHSV(hsv);
                    color.setAlpha(img->GetAlpha(w + x, h + y));

                    current_layer->paint(x + w, y + h, color);
                }
            }
        }
    }
    virtual void penDown()
    {
        just_clicked = true;
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
    bool just_clicked = false;
};

#endif // LIGHTEN_DARKEN
