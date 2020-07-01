#if !defined(COLOR_RAMP_CTRL_H)
#define COLOR_RAMP_CTRL_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/popupwin.h>
#include <vector>

#include "../../color/color.h"
#include "../colorPickers/advColorPicker.hpp"

wxDECLARE_EVENT(EVT_COLOR_RAMP_CHANGE, wxCommandEvent);

/**
 * This calss is a color ramp.
 * emmit this event:
 * EVT_COLOR_RAMP_CHANGE
 */
class ColorRampBar : public wxWindow
{
public:
    ColorRampBar(wxWindow *parent, wxWindowID id = wxID_ANY);
    void add();
    void remove(int index = -1);
    RGBColor mapColor(int value);

    vector<RGBColor> getColorList();
    vector<int> getPosList();

    void init(vector<RGBColor> colors, vector<int> poses);

protected:
    vector<RGBColor> color_list;
    vector<int> position_list;
    int current_selection = 0;

    static const int r = 6;
    static const int h = 20;

    bool need_redraw_bar = true;
    wxImage bar = wxImage(wxSize(1, 1));

    int dragging_index = -1;
    bool clicking = false;

    wxPopupTransientWindow *popup;
    AdvColorPicker *picker;
    int popup_index = -1;

    int insertKnob(int pos, RGBColor color);
    RGBColor getColorAt(int pos);
    RGBColor mixRGB(RGBColor c0, RGBColor c1, double percent);
    void onPaint(wxPaintEvent &event);
    void onSize(wxSizeEvent &event);
    void onMouse(wxMouseEvent &event);
    void onColorPicker(wxCommandEvent &event);
    void moveKnob(int index, int value);

    /** return index of the knob at the position.
     * -1 if in backgroudn
     */
    int posToKnobIndex(int x, int y);
    void sendRampChangeEvent();
    virtual wxSize DoGetBestSize() const override;
};

#endif // COLOR_RAMP_CTRL_H
