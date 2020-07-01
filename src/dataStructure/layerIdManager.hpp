#if !defined(LAYER_ID_MANAGER)
#define LAYER_ID_MANAGER

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <set>

using namespace std;

class LayerIdManager
{
public:
    static void create();
    static LayerIdManager *getInstance();
    static void destroy();

    u_int getId();

    bool isIdAlive(u_int id);

    void deleteId(u_int id);

protected:
    static int instance_count;
    static LayerIdManager *instance;

    u_int next_id = 0;
    set<u_int> id_used;

    LayerIdManager();
    ~LayerIdManager();
};

#endif // LAYER_ID_MANAGER
