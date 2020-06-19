#if !defined(TREE_STRING_DATA_H)
#define TREE_STRING_DATA_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/treectrl.h>
#include <string>

using namespace std;

class TreeStringData : public wxTreeItemData{
    public:
    string dataString;
};


#endif // TREE_STRING_DATA_H
