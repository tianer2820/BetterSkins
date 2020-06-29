#if !defined(ADV_COLOR_PICKER_H)
#define ADV_COLOR_PICKER_H


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/notebook.h>

#include "../../color/color.h"
#include "colorCircle.h"
#include "rgbPanel.h"
#include "hsvPanel.h"
#include "hexPanel.hpp"
#include "colorShower.hpp"
using namespace std;


class AdvColorPicker : public wxPanel, public ColorPicker
{
public:
    AdvColorPicker(wxWindow *parent, wxWindowID id = wxID_ANY);
    virtual Color *getColor();
    virtual void setColor(Color &color);
protected:
    const static int num_pickers = 5;
    ColorPicker *color_picker_list[num_pickers];
    ColorShower *color_shower;
    void onColorChange(wxCommandEvent &event);
    void sendColorChangeEvent();
    virtual wxSize DoGetBestSize() const override;
};

#endif // ADV_COLOR_PICKER_H
