#if !defined(TOOL_H)
#define TOOL_H

#include "../layer.hpp"

#include "map"
#include "vector"
#include "string"
#include "algorithm"

enum class ToolType
{
    PEN,
    COLORPEN,
    DROPPER,
    SELECT,
    MOVE
};

/**
 * Interface class for tools, pens, selectors...
 * 
 * Call setLayer to set the active layer to paint on
 * Call setProperty to set the properties of a pen. Values must be integers.
 * there are some common properties to set, names are capital insensitive:
 * R, G, B, A
 * SIZE
 */
class Tool
{
public:
    void setLayer(Layer *layer)
    {
        current_layer = layer;
    }
    void setProperty(string name, int value)
    {
        for (auto i = name.begin(); i != name.end(); i++)
        {
            *i = toupper(*i); // convert all letters to upper
        }
        properties[name] = value;
    }
    int getProperty(string name)
    {
        for (auto i = name.begin(); i != name.end(); i++)
        {
            *i = toupper(*i);
        }
        auto iter = properties.find(name);
        if (iter == properties.end())
        {
            return -1;
        }
        else
        {
            return (*iter).second;
        }
    }

    virtual ToolType getToolType()
    {
        return tool_type;
    }

    virtual void moveTo(int x, int y) = 0;
    virtual void penDown() = 0;
    virtual void penUp() = 0;
    virtual void setFunctionalKeys(bool shift, bool ctrl) = 0;

    virtual ~Tool() {}

protected:
    Layer *current_layer = NULL;
    bool is_down = false;
    map<string, int> properties;
    vector<string> property_registry;
    ToolType tool_type = ToolType::PEN;
};

#endif // TOOL_H
