#if !defined(RGB_PANEL_H)
#define RGB_PANEL_H


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/spinctrl.h>

#include "../../color/color.h"
#include "colorBars.h"
#include "colorPicker.h"
using namespace std;



class RGBBarPanel : public wxWindow, public ColorPicker
{
public:
    RGBBarPanel(wxWindow *parent, wxWindowID id = wxID_ANY);
    virtual Color *getColor();
    virtual void setColor(Color &color);

protected:
    RGBBar *bar_list[3];
    wxSpinCtrl *spin_list[3];
    void onBarChange(wxCommandEvent &event);
    void onSpinChange(wxSpinEvent &event);
    void sendColorChangeEvent();
};

#endif // RGB_PANEL_H
