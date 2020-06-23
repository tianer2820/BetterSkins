#if !defined(LAYER_HPP)
#define LAYER_HPP

#include <string>
#include <list>
#include <set>

#include "layerModifier.hpp"
#include "command.hpp"
#include "commandManager.hpp"

#include "../imageOperations.hpp"

using namespace std;

// to use set of wxPoint
bool operator>(const wxPoint& point1, const wxPoint& point2){
    if(point1.y != point2.y){
        return point1.y > point2.y;
    } else{
        return point1.x > point2.x;
    }
}
bool operator<(const wxPoint& point1, const wxPoint& point2){
    if(point1.y != point2.y){
        return point1.y < point2.y;
    } else{
        return point1.x < point2.x;
    }
}

/**
 * Each layer is a single "image" that you can paint on or apply modifiers on.
 * There are several properties:
 * name: the name of this layer
 * size: can't be changed, the size of this layer, usually the same as the "skin"
 * image: the actual data object. Do not use it if possible, use render() to get the layer image, use paint and stroke to paint.
 */
class Layer
{
public:
    Layer(string name, wxSize size)
    {
        // initiallize images
        setName(name);
        paint_img = wxImage(size);
        paint_img.InitAlpha();
        clearAlpha(paint_img);

        stroke_img = wxImage(size);
        stroke_img.InitAlpha();
        clearAlpha(stroke_img);
    }
    Layer(const Layer &copy)
    {
        setName(copy.name);
        paint_img = copy.paint_img.Copy();
        stroke_img = copy.stroke_img.Copy();
        stroke_set = copy.stroke_set;
        modifier_list = copy.modifier_list;
        auto c = copy.modifier_list.begin();
        for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
        {
            *i = (*c)->copy();
            c++;
        }
    }
    ~Layer()
    {
        for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
        {
            delete *i; // delete all modifier objects
        }
    }
    string getName()
    {
        return name;
    }
    void setName(string new_name)
    {
        name = new_name;
    }
    /**
     * Don't delete the returned object!
     */
    wxImage *getImage()
    {
        return &paint_img;
    }
    void paint(int x, int y, Color &color)
    {
        // set pixels and dirty pixels here.
        // here the new paint is not really paint onto the image, but to a buffer layer.
        // call stroke to really paint them. stroke will return a command object that is
        // able to undo and redo the paint.
        if (x < 0 || y < 0 || x >= paint_img.GetWidth() || y >= paint_img.GetHeight())
        {
            return; // if out of range, return
        }

        int rgb[3];
        color.getRGB(rgb);
        stroke_img.SetRGB(x, y, rgb[0], rgb[1], rgb[2]);
        stroke_img.SetAlpha(x, y, color.getAlpha());
        // register the changed pixel. They should be updated on next render.
        wxPoint pixel(x, y);

        stroke_set.insert(pixel);
    }
    /**
     * clear the current stroke and dont apply to the layer
     */
    void clearPaint()
    {
        stroke_img.Clear();
        clearAlpha(stroke_img);
        stroke_set.clear();
    }
    /**
     * you need to delete the returned object!
     * Usually you will pass it to the command manager and it will delete it automatically after use
     */
    void stroke()
    {
        // you will need to delete the paint command object manually
        u_char *stroke_data = stroke_img.GetData();
        u_char *img_data = paint_img.GetData();
        int width = paint_img.GetWidth();

        vector<Color *> color_list; // color list required by command

        for (auto i = stroke_set.begin(); i != stroke_set.end(); i++)
        {
            int x = (*i).x;
            int y = (*i).y;
            int index = (y * width + x) * 3;
            // copy rgb
            Color *c = new RGBColor();
            int rgb[3];
            for (int offset = 0; offset < 3; offset++)
            {
                rgb[offset] = img_data[index + offset];
                img_data[index + offset] = stroke_data[index + offset];
            }
            c->setRGB(rgb);

            // copy alpha
            index = y * width + x;
            c->setAlpha(paint_img.GetAlpha()[index]);
            paint_img.GetAlpha()[index] = stroke_img.GetAlpha()[index];

            color_list.push_back(c);
        }
        // create command
        vector<wxPoint> vec;
        vec.assign(stroke_set.begin(), stroke_set.end());
        Command *command = new PaintCommand(getImage(), vec, color_list);
        CommandManager::getInstance()->add(command);
        // empty stroke list
        stroke_set.clear();
    }
    /** 
     * return an image without applying modifiers
     */
    wxImage renderRaw()
    {
        wxImage ret = wxImage(paint_img.GetSize());
        ret.InitAlpha();

        u_char *stroke_data = stroke_img.GetData();
        u_char *stroke_alpha = stroke_img.GetAlpha();
        u_char *img_data = paint_img.GetData();
        u_char *img_alpha = paint_img.GetAlpha();
        u_char *canvas_data = ret.GetData();
        u_char *canvas_alpha = ret.GetAlpha();
        int width = paint_img.GetWidth();
        int height = paint_img.GetHeight();
        // fill canvas with image
        for (int i = 0; i < width * height; i++)
        {
            // fill alpha
            canvas_alpha[i] = img_alpha[i];
            // fill rgb
            for (int offset = 0; offset < 3; offset++)
            {
                canvas_data[i * 3 + offset] = img_data[i * 3 + offset];
            }
        }
        // apply the stroke
        for (auto i = stroke_set.begin(); i != stroke_set.end(); i++)
        {
            int x = (*i).x;
            int y = (*i).y;
            int index = (y * width + x);
            // replace each pixel
            for (int c = 0; c < 3; c++)
            {
                canvas_data[index * 3 + c] = stroke_data[index * 3 + c];
            }
            canvas_alpha[index] = stroke_alpha[index];
        }

        return ret;
    }
    /**
     * get the final render for this layer, with the modifiers applied
     */
    wxImage render()
    {
        // render the raw color
        wxImage raw = this->renderRaw();

        // apply all modifiers
        for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
        {
            LayerModifier *modifier = *i;
            modifier->render(raw);
        }
        return raw;
    }
    /**
     * the returned list is only for reading, editing it will not affact the origional list
     */
    vector<LayerModifier *> getModifierList()
    {
        return modifier_list;
    }
    /**
     * the layer will delete the object automatically.
     */
    void addModifier(LayerModifier *modifier)
    {
        modifier_list.push_back(modifier);
    }
    void deleteModifier(int index)
    {
        LayerModifier *modifier = modifier_list.at(index);
        modifier_list.erase(modifier_list.begin() + index);
        delete modifier;
    }
    void moveModefier(int from, int to)
    {
        LayerModifier *modifier = modifier_list.at(from);
        modifier_list.erase(modifier_list.begin() + from);
        modifier_list.insert(modifier_list.begin() + to, modifier);
    }

    void setVisable(bool visable = true){
        this->visable = visable;
    }
    bool getVisable(){
        return visable;
    }
protected:
    string name;
    bool visable = true;
    wxImage paint_img;
    wxImage stroke_img;
    set<wxPoint> stroke_set;
    vector<LayerModifier *> modifier_list;

};

#endif // LAYER_HPP
