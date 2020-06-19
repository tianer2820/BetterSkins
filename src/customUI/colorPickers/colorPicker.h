
#if !defined(COLOR_PICKER_H)
#define COLOR_PICKER_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../color/color.h"

wxDECLARE_EVENT(EVT_COLOR_PICKER_CHANGE, wxCommandEvent);

class ColorPicker{
    public:
    virtual Color* getColor() = 0; // You MUST Delete the returned object after use!!!
    virtual void setColor(Color& color) = 0;
};

#endif // COLOR_PICKER_H

