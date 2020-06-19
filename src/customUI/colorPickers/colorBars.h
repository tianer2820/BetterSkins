#if !defined(COLOR_BARS_H)
#define COLOR_BARS_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "../../color/color.h"

wxDECLARE_EVENT(EVT_COLOR_BAR_CHANGE, wxCommandEvent);


class ColorBar : public wxWindow
{
public:
    ColorBar(wxWindow *parent, wxWindowID id = wxID_ANY, int gradiaentChannel = 0);
    void setChannel(int channel, int value);
    void setValue(int value);
    void setColor(int *color);
    int getValue();

protected:
    const int channel;
    int channelMax = 255;
    int color[3] = {0, 0, 0};
    bool needRedraw = true;
    bool dragging = false;
    wxImage img;

    void onSize(wxSizeEvent &event);
    void onMouse(wxMouseEvent &event);
    void sendValueChangeEvent();
    void onPaint(wxPaintEvent &event);
    void drawImg(wxImage &image);
    virtual void colorSpaceToRGB(int *color, int *out) = 0;

    virtual wxSize DoGetBestSize() const override;
};

class RGBBar : public ColorBar
{
public:
    RGBBar(wxWindow *parent, wxWindowID id = wxID_ANY, int gradiaentChannel = 0);
protected:
    virtual void colorSpaceToRGB(int *color, int *out);
};

class HSVBar : public ColorBar
{
public:
    HSVBar(wxWindow *parent, wxWindowID id = wxID_ANY, int gradiaentChannel = 0);
protected:
    virtual void colorSpaceToRGB(int *color, int *out);
};

#endif // COLOR_BARS_H
