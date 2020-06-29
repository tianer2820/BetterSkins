#if !defined(COLOR_SHOWER_H)
#define COLOR_SHOWER_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>

#include "../../color/color.h"
#include "../../imageOperations.hpp"

class ColorShower : public wxWindow
{
public:
    ColorShower(wxWindow *parent, wxWindowID id = wxID_ANY);
    void setColor(int* rgb, int alpha);

protected:
    int rgba[4] = {0, 0, 0, 255};
    wxImage bg_checker;
    wxImage final_render;
    bool need_redraw_bg = true;
    bool need_redraw_final_render = true;

    void onSize(wxSizeEvent& event);
    void onPaint(wxPaintEvent& event);
};

#endif // COLOR_SHOWER_H
