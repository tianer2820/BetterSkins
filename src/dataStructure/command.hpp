#ifndef COMMAND_H
#define COMMAND_H


#include <string>


using namespace std;


class Command
{
public:
    virtual void undo() = 0;
    virtual void redo() = 0;
    bool isDone()
    {
        return is_done;
    }
    string getName()
    {
        return name;
    }
    virtual ~Command(){
    }

protected:
    bool is_done = true;
    string name = "Empty Command";
};

class PaintCommand : public Command
{
public:
    /**
    * this class will delete the color objects automatically
    * the length of the two list should be the same
    */
    PaintCommand(wxImage *img, vector<wxPoint> point_list, vector<Color *> color_list)
    {
        image = img;
        this->point_list = point_list;
        this->color_list = color_list;
        is_done = true; // assume the operation is done.
    }
    ~PaintCommand()
    {
        for (auto i = color_list.begin(); i != color_list.end(); i++)
        {
            delete (*i);
        }
    }
    virtual void undo()
    {
        if (!isDone())
        {
            return;
        }
        flipPixels();
        is_done = false;
    }
    virtual void redo()
    {
        if (isDone())
        {
            return;
        }
        flipPixels();
        is_done = true;
    }

protected:
    wxImage *image;
    vector<wxPoint> point_list;
    vector<Color *> color_list;
    void flipPixels()
    {
        assert(point_list.size() == color_list.size());
        int length = point_list.size();
        auto point = point_list.begin();
        auto color = color_list.begin();
        for (int i = 0; i < length; i++)
        {
            int x = (*point).x;
            int y = (*point).y;
            // old color from img
            int current_rgb[3];
            int current_a = image->GetAlpha(x, y);
            current_rgb[0] = image->GetRed(x, y);
            current_rgb[1] = image->GetGreen(x, y);
            current_rgb[2] = image->GetBlue(x, y);
            // new color from list
            int new_rgb[3];
            int new_a = (*color)->getAlpha();
            (*color)->getRGB(new_rgb);
            // set colors
            (*color)->setRGB(current_rgb);
            (*color)->setAlpha(current_a);
            image->SetRGB(x, y, new_rgb[0], new_rgb[1], new_rgb[2]);
            image->SetAlpha(x, y, new_a);
            // increase iterators
            point++;
            color++;
        }
    }
};


#endif