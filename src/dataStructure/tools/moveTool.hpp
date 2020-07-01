#if !defined(MOVE_TOOL_H)
#define MOVE_TOOL_H

#include "Tool.hpp"
#include "../layer.hpp"
#include "../../color/color.h"

/**
 * Moves the whole layer.
 */
class MoveTool : public Tool
{
public:
    MoveTool()
    {
        setProperty("R", 0);
        setProperty("G", 0);
        setProperty("b", 0);
        setProperty("a", 255);
        setProperty("SIZE", 1);
        tool_type = ToolType::MOVE;
    }
    virtual void moveTo(int x, int y)
    {
        if(this->x == x && this->y == y){
            return; // not moved
        }
        this->x = x;
        this->y = y;
        if (is_down && current_layer != NULL)
        {
            int w = image_cache.GetWidth();
            int h = image_cache.GetHeight();

            int offset_x = x - start_x;
            int offset_y = y - start_y;

            for (int c_y = 0; c_y < h; c_y++)
            {
                for (int c_x = 0; c_x < w; c_x++)
                {
                    int map_to_x = c_x - offset_x;
                    int map_to_y = c_y - offset_y;
                    if(map_to_x < 0 || map_to_y < 0 || map_to_x >= w || map_to_y >= h){
                        // if the pixel maps to outside the cache
                        current_layer->paint(c_x, c_y, RGBColor(0, 0, 0, 0));
                        continue;
                    }
                    // if the pixel maps to a correct pixel
                    current_layer->paint(c_x, c_y,
                    RGBColor(image_cache.GetRed(map_to_x, map_to_y),
                    image_cache.GetGreen(map_to_x, map_to_y),
                    image_cache.GetBlue(map_to_x, map_to_y),
                    image_cache.GetAlpha(map_to_x, map_to_y)));
                }
            }

        }
    }
    virtual void penDown()
    {
        if (current_layer != nullptr)
        {
            is_down = true;
            start_x = x; // store initial position
            start_y = y;
            image_cache = current_layer->getImage()->Copy();
            moveTo(x, y);
        }
    }
    virtual void penUp()
    {
        if (is_down && current_layer != NULL)
        {
            current_layer->stroke();
        }
        is_down = false;
    }
    
    void mirrorX(){
        if(current_layer == nullptr){
            return;
        }
        wxImage img = current_layer->getImage()->Mirror(true);
        current_layer->getImage()->Paste(img, 0, 0);
    }
    void mirrorY(){
        if(current_layer == nullptr){
            return;
        }
        wxImage img = current_layer->getImage()->Mirror(false);
        current_layer->getImage()->Paste(img, 0, 0);
    }
    
    virtual void setFunctionalKeys(bool shift, bool ctrl)
    {
    }

protected:
    int x, y;
    int start_x, start_y;
    wxImage image_cache;
};

#endif // MOVE_TOOL_H
