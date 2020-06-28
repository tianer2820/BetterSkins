#if !defined(LAYER_ID_MANAGER)
#define LAYER_ID_MANAGER

#include <set>

class LayerIdManager
{
public:
    static void create()
    {
        if (instance_count == 0)
        {
            instance = new LayerIdManager();
            instance_count += 1;
        }
    }
    static LayerIdManager *getInstance()
    {
        if (instance_count == 0)
        {
            return nullptr;
        }
        return instance;
    }
    static void destroy()
    {
        if (instance_count > 0)
        {
            delete instance;
        }
    }

    u_int getId()
    {
        int ret = next_id;
        id_used.insert(next_id);
        next_id += 1;
        return ret;
    }

    bool isIdAlive(u_int id)
    {
        return id_used.count(id) == 1;
    }

    void deleteId(u_int id)
    {
        id_used.erase(id);
    }

protected:
    static int instance_count;
    static LayerIdManager *instance;

    u_int next_id = 0;
    set<u_int> id_used;

    LayerIdManager()
    {
    }
    ~LayerIdManager()
    {
    }
};

int LayerIdManager::instance_count = 0;
LayerIdManager* LayerIdManager::instance = nullptr;

#endif // LAYER_ID_MANAGER
