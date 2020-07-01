#include "layerIdManager.hpp"
#include <set>

void LayerIdManager::create()
{
    if (instance_count == 0)
    {
        instance = new LayerIdManager();
        instance_count += 1;
    }
}
LayerIdManager *LayerIdManager::getInstance()
{
    if (instance_count == 0)
    {
        return nullptr;
    }
    return instance;
}
void LayerIdManager::destroy()
{
    if (instance_count > 0)
    {
        delete instance;
    }
}

u_int LayerIdManager::getId()
{
    int ret = next_id;
    id_used.insert(next_id);
    next_id += 1;
    return ret;
}

bool LayerIdManager::isIdAlive(u_int id)
{
    return id_used.count(id) == 1;
}

void LayerIdManager::deleteId(u_int id)
{
    id_used.erase(id);
}

LayerIdManager::LayerIdManager()
{
}
LayerIdManager::~LayerIdManager()
{
}

int LayerIdManager::instance_count = 0;
LayerIdManager *LayerIdManager::instance = nullptr;
