#ifndef SKIN_H
#define SKIN_H

#include <vector>
#include <string>

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
    SkinType getSkinType(){
        return this->skin_type;
    }
    // file operations
    void loadFile(string filename)
    {
        throw("Unimplimented");
    }
    void saveFile(string filename)
    {
        throw("Unimplimented");
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
     */
    void addLayer(Layer *layer, int index = -1)
    {
        if (index == -1)
        {
            index = layers.size();
        }
        layers.insert(layers.begin() + index, layer);
    }
    void deleteLayer(int index)
    {
        Layer *layer = layers.at(index);
        layers.erase(layers.begin() + index);
        delete layer;
    }
    void moveLayer(int from, int to)
    {
        Layer *layer = layers.at(from);
        layers.erase(layers.begin() + from);
        layers.insert(layers.begin() + to, layer);
    }
    // properties
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
            layer->render(render);
        }
        return render;
    }

protected:
    string name;
    vector<Layer *> layers;
    SkinType skin_type;
};


#endif