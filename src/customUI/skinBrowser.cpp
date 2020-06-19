#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "treeStringData.h"
#include "skinBrowser.h"
#include <wx/treectrl.h>
#include <wx/dir.h>
#include <wx/filename.h>

using namespace std;

wxDEFINE_EVENT(EVT_SKINBROWSER_OPEN_FILE, wxCommandEvent);

SkinBrowser::SkinBrowser(wxWindow *parent, wxWindowID id) : wxTreeCtrl(parent, id)
{
    Bind(wxEVT_TREE_ITEM_ACTIVATED, &SkinBrowser::onClick, this);
}
void SkinBrowser::openDir(string dir, string fileFilter[])
{
    wxString wxdir = wxString::FromUTF8(dir);
    assert(wxDirExists(wxdir));
    this->DeleteAllItems();
    wxFileName name(wxdir);
    name.MakeAbsolute();
    wxdir = name.GetFullPath();

    wxArrayString fileList;
    wxDir::GetAllFiles(wxdir, &fileList, wxEmptyString, wxDIR_FILES);
    TreeStringData *d = new TreeStringData;
    d->dataString = wxdir;
    wxTreeItemId root = this->AddRoot(wxdir, -1, -1, d);
    this->loadFileList(root, wxdir.ToStdString(), fileFilter);
}

void SkinBrowser::loadFileList(wxTreeItemId rootNode, string rootPath, string fileType[])
{
    wxDir dir(rootPath);
    wxString str;
    bool ret = dir.GetFirst(&str, wxEmptyString, wxDIR_DIRS);
    while (ret)
    {
        wxTreeItemId node = this->AppendItem(rootNode, str);
        this->loadFileList(node, rootPath + str.ToStdString() + "/", fileType);
        ret = dir.GetNext(&str);
    }
    ret = dir.GetFirst(&str, wxEmptyString, wxDIR_FILES);
    while (ret)
    {
        TreeStringData *d = new TreeStringData;
        d->dataString = rootPath + str.ToStdString();
        this->AppendItem(rootNode, str, -1, -1, d);
        ret = dir.GetNext(&str);
    }
}
bool SkinBrowser::isFileType(string fileName, string fileTypes[])
{
    if (fileTypes == NULL)
    {
        return true;
    }
    return false;
}
void SkinBrowser::onClick(wxTreeEvent &event)
{
    wxTreeItemId id = event.GetItem();
    TreeStringData *data = (TreeStringData *)this->GetItemData(id);
    wxCommandEvent *e = new wxCommandEvent(EVT_SKINBROWSER_OPEN_FILE, this->GetId());
    e->SetString(wxString::FromUTF8(data->dataString));
    wxQueueEvent(GetEventHandler(), e);
}
