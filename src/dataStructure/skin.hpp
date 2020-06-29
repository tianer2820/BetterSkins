#ifndef SKIN_H
#define SKIN_H

#include <vector>
#include <string>

#include <json.hpp>
using json = nlohmann::json;

#include "layer.hpp"
#include "../imageOperations.hpp"

using namespace std;

enum SkinType
{
    STEVE,
    ALEX,
    STEVE_MIN,
    ALEX_MIN
};

class Skin
{
public:
    Skin(string name, SkinType type = STEVE)
    {
        this->name = name;
        skin_type = type;
    }
    ~Skin()
    {
        for (auto i = layers.begin(); i != layers.end(); i += 1)
        {
            delete *i; // delete all layers
        }
    }
    SkinType getSkinType()
    {
        return this->skin_type;
    }
    // file operations
    /**
     * return false if json cant be loaded
     */
    bool loadJson(json j)
    {
        if (!j.is_object())
        {
            return false;
        }
        // name
        if (j.contains("name"))
        {
            name = j["name"].get<string>();
        }
        else
        {
            return false;
        }
        // skin types
        if (j.contains("skin_type"))
        {
            string t = j["skin_type"];
            if (t == "STEVE")
            {
                skin_type = SkinType::STEVE;
            }
            else if (t == "ALEX")
            {
                skin_type = SkinType::ALEX;
            }
            else if (t == "STEVE_MIN")
            {
                skin_type = SkinType::STEVE_MIN;
            }
            else if (t == "ALEX_MIN")
            {
                skin_type = SkinType::ALEX_MIN;
            }
            else
            {
                return false;
            }
            // layers
            if (j.contains("layers") && j["layers"].is_array())
            {
                json json_layers = j["layers"];
                int length = json_layers.size();
                for (int i = 0; i < length; i++)
                {
                    Layer *layer = new Layer("layer", getLayerSize());
                    this->addLayer(layer);
                    bool success = layer->loadJson(json_layers[i]);
                    if (!success)
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
        setModified(false);
        return true;
    }
    json toJson()
    {
        json j;
        j["name"] = name;
        switch (skin_type)
        {
        case SkinType::STEVE:
            j["skin_type"] = "STEVE";
            break;
        case SkinType::ALEX:
            j["skin_type"] = "ALEX";
            break;
        case SkinType::STEVE_MIN:
            j["skin_type"] = "STEVE_MIN";
            break;
        case SkinType::ALEX_MIN:
            j["skin_type"] = "ALEX_MIN";
            break;

        default:
            break;
        }
        j["layers"] = json::array();
        for (auto i = layers.begin(); i != layers.end(); i++)
        {
            Layer *layer = *i;
            j["layers"].push_back(layer->toJson());
        }
        return j;
    }
    bool isModified()
    {
        if (modified)
        {
            return true;
        }
        for (auto i = layers.begin(); i != layers.end(); i++)
        {
            if ((*i)->isModified())
            {
                return true;
            }
        }
        return false;
    }
    void setModified(bool v)
    {
        modified = v;
        if (!v)
        {
            for (auto i = layers.begin(); i != layers.end(); i++)
            {
                (*i)->setModified(false);
            }
        }
    }

    // layer operations
    /**
     * dont delete the returned
     */
    Layer *getLayer(int index)
    {
        return layers.at(index);
    }
    int getLayerNum()
    {
        return layers.size();
    }
    /**
     * this class will delete layers automatically
     * leave the index as -1 to append the new layer
     */
    void addLayer(Layer *layer, int index = -1)
    {
        if (index == -1)
        {
            index = layers.size();
        }
        layers.insert(layers.begin() + index, layer);
        modified = true;
    }
    void deleteLayer(int index)
    {
        Layer *layer = layers.at(index);
        layers.erase(layers.begin() + index);
        delete layer;
        modified = true;
    }
    void moveLayer(int from, int to)
    {
        Layer *layer = layers.at(from);
        layers.erase(layers.begin() + from);
        layers.insert(layers.begin() + to, layer);
        modified = true;
    }
    // properties
    /**
     * return the best layer size based on the skin type
     */
    wxSize getLayerSize()
    {
        if (skin_type == STEVE || skin_type == ALEX)
        {
            return wxSize(64, 64);
        }
        return wxSize(64, 32);
    }
    // rendering
    wxImage render()
    {
        // render the skin into a image. use this for display
        wxImage render(getLayerSize());
        render.InitAlpha();
        clearAlpha(render);
        for (auto i = layers.begin(); i != layers.end(); i++)
        {
            Layer *layer = *i;
            if (!layer->getVisible())
            {
                continue;
            }
            wxImage layer_render = layer->render();
            alphaOver(render, layer_render);
        }
        return render;
    }

protected:
    string name;
    vector<Layer *> layers;
    SkinType skin_type;
    bool modified = true;
};

#endif