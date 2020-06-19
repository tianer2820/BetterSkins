
#if !defined(COLOR_SQUARE_H)
#define COLOR_SQUARE_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../color/color.h"
#include "colorPicker.h"


class ColorSquare : public wxWindow, public ColorPicker
{
public:
    ColorSquare(wxWindow *parent, wxWindowID id = wxID_ANY);

    virtual Color *getColor();
    virtual void setColor(Color &color);

protected:
    HSVColor current_color;
    bool need_redraw_bar = true;
    bool need_redraw_square = true;
    wxImage current_image = wxImage(1, 1);
    bool dragging_square = false;
    bool dragging_bar = false;

    void onMouse(wxMouseEvent &event);

    void onSize(wxSizeEvent &event);

    void onPaint(wxPaintEvent &event);

    void drawImage();

    void sendColorPickerUpdateEvent();

    virtual wxSize DoGetBestSize() const override;
};

#endif // COLOR_SQUARE_H
