#if !defined(HEX_PANEL_H)
#define HEX_PANEL_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../color/color.h"
#include "colorPicker.h"
using namespace std;


class HEXPanel : public wxPanel, public ColorPicker
{
public:
    HEXPanel(wxWindow *parent, wxWindowID id = wxID_ANY);
    virtual Color *getColor();
    virtual void setColor(Color &color);
protected:
    wxTextCtrl* entry;
    void onTextChange(wxCommandEvent& event);
};

#endif // HEX_PANEL_H
