#include "layer.hpp"

#include <string>
#include <list>
#include <set>

#include <json.hpp>
using json = nlohmann::json;
#include <base64.h>

#include "layerModifier.hpp"
#include "command.hpp"
#include "commandManager.hpp"
#include "layerIdManager.hpp"

#include "../imageOperations.hpp"

using namespace std;

// to use set of wxPoint
bool operator>(const wxPoint &point1, const wxPoint &point2)
{
    if (point1.y != point2.y)
    {
        return point1.y > point2.y;
    }
    else
    {
        return point1.x > point2.x;
    }
}
bool operator<(const wxPoint &point1, const wxPoint &point2)
{
    if (point1.y != point2.y)
    {
        return point1.y < point2.y;
    }
    else
    {
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

Layer::Layer(string name, wxSize size)
{
    // initiallize images
    setName(name);
    paint_img = wxImage(size);
    paint_img.InitAlpha();
    clearAlpha(paint_img);

    stroke_img = wxImage(size);
    stroke_img.InitAlpha();
    clearAlpha(stroke_img);

    id = LayerIdManager::getInstance()->getId();
}
Layer::Layer(const Layer &copy)
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
    id = LayerIdManager::getInstance()->getId();
}
Layer::~Layer()
{
    for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
    {
        delete *i; // delete all modifier objects
    }
    LayerIdManager::getInstance()->deleteId(id);
}
u_int Layer::getId()
{
    return id;
}
string Layer::getName()
{
    return name;
}
void Layer::setName(string new_name)
{
    name = new_name;
    modified = true;
}
/**
     * Don't delete the returned object!
     */
wxImage *Layer::getImage()
{
    return &paint_img;
}
void Layer::paint(int x, int y, const Color &color)
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
void Layer::clearPaint()
{
    stroke_img.Clear();
    clearAlpha(stroke_img);
    stroke_set.clear();
}
/**
     * you need to delete the returned object!
     * Usually you will pass it to the command manager and it will delete it automatically after use
     */
void Layer::stroke()
{
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
    Command *command = new PaintCommand(this, getImage(), vec, color_list);
    CommandManager::getInstance()->add(command);
    // empty stroke list
    stroke_set.clear();
    modified = true;
}
/** 
     * return an image without applying modifiers
     */
wxImage Layer::renderRaw()
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
wxImage Layer::render()
{
    // render the raw color
    wxImage raw = this->renderRaw();

    // apply all modifiers
    for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
    {
        LayerModifier *modifier = *i;
        if (!modifier->getVisible())
        {
            continue;
        }
        modifier->render(raw);
    }
    return raw;
}
/**
     * the returned list is only for reading, editing it will not affact the origional list
     */
vector<LayerModifier *> Layer::getModifierList()
{
    return modifier_list;
}
/**
     * the layer will delete the object automatically.
     */
void Layer::addModifier(LayerModifier *modifier)
{
    modifier_list.push_back(modifier);
    modified = true;
}
void Layer::deleteModifier(int index)
{
    LayerModifier *modifier = modifier_list.at(index);
    modifier_list.erase(modifier_list.begin() + index);
    delete modifier;
    modified = true;
}
void Layer::moveModefier(int from, int to)
{
    LayerModifier *modifier = modifier_list.at(from);
    modifier_list.erase(modifier_list.begin() + from);
    modifier_list.insert(modifier_list.begin() + to, modifier);
    modified = true;
}

void Layer::setVisible(bool visible)
{
    this->visible = visible;
    modified = true;
}
bool Layer::getVisible()
{
    return visible;
}

json Layer::toJson()
{
    json j;
    j["layer_name"] = name;
    j["visible"] = visible;

    wxImage raw = renderRaw();
    u_char *rgb_data = raw.GetData();
    u_char *alpha_data = raw.GetAlpha();
    int size = raw.GetWidth() * raw.GetHeight();
    j["rgb"] = base64_encode(rgb_data, size * 3);
    j["alpha"] = base64_encode(alpha_data, size);

    j["modifiers"] = json::array();
    for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
    {
        LayerModifier *modifier = *i;
        j["modifiers"].push_back(modifier->toJson());
    }
    return j;
}
bool Layer::loadJson(json j)
{
    if (!j.is_object())
    {
        return false;
    }

    if (j.contains("layer_name"))
    {
        name = j["layer_name"].get<string>();
    }
    else
    {
        return false;
    }
    if (j.contains("visible"))
    {
        visible = j["visible"];
    }
    else
    {
        return false;
    }
    if (j.contains("rgb") && j.contains("alpha"))
    {
        string rgb_str = base64_decode(j["rgb"]);
        const char *rgb_data = rgb_str.data();
        string alpha_str = base64_decode(j["alpha"]);
        const char *alpha_data = alpha_str.data();

        u_char *data = paint_img.GetData();
        u_char *alpha = paint_img.GetAlpha();
        int size = paint_img.GetWidth() * paint_img.GetHeight();
        for (int i = 0; i < size; i++)
        {
            for (int c = 0; c < 3; c++)
            {
                data[i * 3 + c] = rgb_data[i * 3 + c];
            }
            alpha[i] = alpha_data[i];
        }
    }
    else
    {
        return false;
    }
    if (j.contains("modifiers") && j["modifiers"].is_array())
    {
        json modifiers = j["modifiers"];
        int length = modifiers.size();
        for (int i = 0; i < length; i++)
        {
            string name = modifiers[i]["modifier_name"];
            LayerModifier *new_modifier;
            if (name == "Gray")
            {
                new_modifier = new LayerGrayModifier();
            }
            else if (name == "Alpha Blend")
            {
                new_modifier = new LayerAlphaBlendModifier();
            }
            else if (name == "Color Ramp")
            {
                new_modifier = new LayerColorRampModifier();
            }
            else
            {
                return false;
            }
            bool successful = new_modifier->loadJson(modifiers[i]);
            if (!successful)
            {
                delete new_modifier;
                return false;
            }
            this->addModifier(new_modifier);
        }
    }
    else
    {
        return false;
    }
    modified = false;
    return true;
}
bool Layer::isModified()
{
    if (modified)
    {
        return true;
    }
    for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
    {
        if ((*i)->isModified())
        {
            return true;
        }
    }
    return false;
}
void Layer::setModified(bool v)
{
    modified = v;
    if (!v)
    {
        for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
        {
            (*i)->setModified(false);
        }
    }
}
