#if !defined(SKIN_BROWSER_H)
#define SKIN_BROWSER_H


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/treectrl.h>

using namespace std;

wxDECLARE_EVENT(EVT_SKINBROWSER_OPEN_FILE, wxCommandEvent);

class SkinBrowser : public wxTreeCtrl{
    public:
    SkinBrowser(wxWindow *parent, wxWindowID id=wxID_ANY);
    void openDir(string dir, string fileFilter[]=NULL);
    protected:
    void loadFileList(wxTreeItemId rootNode, string rootPath, string fileType[]);
    bool isFileType(string fileName, string fileTypes[]);
    void onClick(wxTreeEvent &event);

};



#endif // SKIN_BROWSER_H
