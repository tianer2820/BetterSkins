#if !defined(HSV_PANEL_H)
#define HSV_PANEL_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/spinctrl.h>

#include "../../color/color.h"
#include "colorBars.h"
#include "colorPicker.h"
using namespace std;



class HSVBarPanel : public wxWindow, public ColorPicker
{
public:
    HSVBarPanel(wxWindow *parent, wxWindowID id = wxID_ANY);
    virtual Color *getColor();
    virtual void setColor(Color &color);

protected:
    HSVBar *barList[3];
    wxSpinCtrl *spinList[3];
    void onBarChange(wxCommandEvent &event);
    void onSpinChange(wxSpinEvent &event);
    void sendColorChangeEvent();
};

#endif // HSV_PANEL_H
