#if !defined(LAYER_HPP)
#define LAYER_HPP

#include <string>
#include <list>
#include <set>

#include <json.hpp>
using json = nlohmann::json;

#include "layerModifier.hpp"

using namespace std;

// to use set of wxPoint
bool operator>(const wxPoint &point1, const wxPoint &point2);
bool operator<(const wxPoint &point1, const wxPoint &point2);

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
    Layer(string name, wxSize size);
    Layer(const Layer &copy);
    ~Layer();
    u_int getId();
    string getName();
    void setName(string new_name);
    /**
     * Don't delete the returned object!
     */
    wxImage *getImage();
    void paint(int x, int y, Color &color);
    /**
     * clear the current stroke and dont apply to the layer
     */
    void clearPaint();
    /**
     * you need to delete the returned object!
     * Usually you will pass it to the command manager and it will delete it automatically after use
     */
    void stroke();
    /** 
     * return an image without applying modifiers
     */
    wxImage renderRaw();
    /**
     * get the final render for this layer, with the modifiers applied
     */
    wxImage render();
    /**
     * the returned list is only for reading, editing it will not affact the origional list
     */
    vector<LayerModifier *> getModifierList();
    /**
     * the layer will delete the object automatically.
     */
    void addModifier(LayerModifier *modifier);
    void deleteModifier(int index);
    void moveModefier(int from, int to);

    void setVisible(bool visible = true);
    bool getVisible();

    json toJson();
    bool loadJson(json j);
    bool isModified();
    void setModified(bool v);


protected:
    string name;
    bool visible = true;
    wxImage paint_img;
    wxImage stroke_img;
    set<wxPoint> stroke_set;
    vector<LayerModifier *> modifier_list;
    u_int id;

    bool modified = true;
};

#endif // LAYER_HPP
