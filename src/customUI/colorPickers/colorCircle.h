#ifndef COLOR_CIRCLE_H
#define COLOR_CIRCLE_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "../../color/color.h"
#include "colorPicker.h"


class ColorCircle : public wxPanel, public ColorPicker
{
public:
    ColorCircle(wxWindow *parent, wxWindowID id = wxID_ANY);
    void setColor(Color& newColor);
    Color* getColor();

protected:
    int hsv[3] = {0, 0, 0};
    bool draging_circle = false;
    bool draging_triangle = false;
    bool need_redraw_circle = true;
    bool need_redraw_triangle = true;
    wxImage last_image;

    virtual wxSize DoGetBestSize() const override;
    void onSize(wxSizeEvent &event);
    void onPaint(wxPaintEvent &event);
    void onMouse(wxMouseEvent &event);

    void paintImage(wxImage &image);
    int checkArea(int x, int y);
};

#endif // COLOR_CIRCLE_H
