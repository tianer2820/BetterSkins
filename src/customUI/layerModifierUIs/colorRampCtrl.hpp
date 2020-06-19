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
wxDEFINE_EVENT(EVT_COLOR_RAMP_CHANGE, wxCommandEvent);

/**
 * This calss is a color ramp.
 * emmit this event:
 * EVT_COLOR_RAMP_CHANGE
 */
class ColorRampBar : public wxWindow
{
public:
    ColorRampBar(wxWindow *parent, wxWindowID id = wxID_ANY) : wxWindow(parent, id)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);

        Bind(wxEVT_PAINT, &ColorRampBar::onPaint, this);
        Bind(wxEVT_SIZE, &ColorRampBar::onSize, this);

        Bind(wxEVT_LEFT_DOWN, &ColorRampBar::onMouse, this);
        Bind(wxEVT_MOTION, &ColorRampBar::onMouse, this);
        Bind(wxEVT_LEFT_UP, &ColorRampBar::onMouse, this);

        insertKnob(0, RGBColor(0, 0, 0));
        insertKnob(255, RGBColor(255, 255, 255));

        popup = new wxPopupTransientWindow(this);
        popup->SetWindowStyle(wxPU_CONTAINS_CONTROLS);

        picker = new AdvColorPicker(popup);
        wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
        box->Add(picker, 1, wxEXPAND);
        popup->SetSizer(box);
        popup->SetSize(200, 400);
        popup->Bind(EVT_COLOR_PICKER_CHANGE, &ColorRampBar::onColorPicker, this, picker->GetId());
    }

    void add()
    {
        if (color_list.size() <= 1)
        {
            color_list.push_back(getColorAt(255 / 2));
            position_list.push_back(255 / 2);
            need_redraw_bar = true;
            Refresh();
            return;
        }
        int index = current_selection;
        if (current_selection == color_list.size() - 1)
        {
            index -= 1;
        }

        int pos = (position_list.at(index) + position_list.at(index + 1)) / 2;
        this->insertKnob(pos, getColorAt(pos));

        need_redraw_bar = true;
        Refresh();
        sendRampChangeEvent();
    }
    void remove(int index = -1)
    {
        if (index == -1)
        {
            index = current_selection;
        }
        if (index < 0 || index > position_list.size() - 1)
        {
            return;
        }
        position_list.erase(position_list.begin() + index);
        color_list.erase(color_list.begin() + index);
        need_redraw_bar = true;
        Refresh();
        current_selection -= 1;
        sendRampChangeEvent();
    }
    RGBColor mapColor(int value){
        return getColorAt(value);
    }

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

    int insertKnob(int pos, RGBColor color)
    {
        for (int i = 0; i < position_list.size(); i++)
        {
            if (position_list.at(i) >= pos)
            {
                position_list.insert(position_list.begin() + i, pos);
                color_list.insert(color_list.begin() + i, color);
                need_redraw_bar = true;
                Refresh();
                return i;
            }
        }
        position_list.push_back(pos);
        color_list.push_back(color);
        need_redraw_bar = true;
        Refresh();
        return position_list.size() - 1;
    }

    RGBColor getColorAt(int pos)
    {
        if (color_list.size() < 1)
        {
            return RGBColor(0, 0, 0);
        }
        int index = position_list.size() - 1;
        for (int i = 0; i < position_list.size(); i++)
        {
            int value = position_list.at(i);
            if (value >= pos)
            {
                index = i - 1;
                break;
            }
        }
        if (index < 0)
        {
            // before first
            return color_list.at(0);
        }
        if (index >= position_list.size() - 1)
        {
            // after last
            return color_list.at(position_list.size() - 1);
        }
        // in middle
        RGBColor c0 = color_list.at(index);
        RGBColor c1 = color_list.at(index + 1);
        int pos0 = position_list.at(index);
        int pos1 = position_list.at(index + 1);
        double mix_percent = static_cast<double>(pos - pos0) / static_cast<double>(pos1 - pos0);
        return mixRGB(c0, c1, mix_percent);
    }

    RGBColor mixRGB(RGBColor c0, RGBColor c1, double percent)
    {
        int rgb0[3];
        int rgb1[3];
        c0.getRGB(rgb0);
        c1.getRGB(rgb1);
        int out[3];
        for (int i = 0; i < 3; i++)
        {
            out[i] = rgb0[i] * (1 - percent) + rgb1[i] * percent;
        }
        int alpha = c0.getAlpha() * percent + c1.getAlpha() * (1 - percent);
        RGBColor c_out;
        c_out.setRGB(out);
        c_out.setAlpha(alpha);
        return c_out;
    }

    void onPaint(wxPaintEvent &event)
    {
        wxRect bar_area;
        bar_area.x = r;
        bar_area.y = r;
        bar_area.width = bar.GetWidth() - 2 * r;
        bar_area.height = h;

        u_char *rgb_data = bar.GetData();
        int offset = 0;
        wxColor bg_color;
        bg_color = GetBackgroundColour();
        int bg_rgb[3] = {bg_color.Red(), bg_color.Green(), bg_color.Blue()};

        if (need_redraw_bar)
        {
            need_redraw_bar = false;
            for (int y = 0; y < bar.GetHeight(); y++)
            {
                for (int x = 0; x < bar.GetWidth(); x++)
                {
                    if (bar_area.Contains(x, y))
                    {
                        RGBColor color = getColorAt((x - bar_area.x) * 255 / bar_area.width);
                        int rgb[3];
                        color.getRGB(rgb);
                        for (int i = 0; i < 3; i++)
                        {
                            rgb_data[offset + i] = rgb[i];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            rgb_data[offset + i] = bg_rgb[i];
                        }
                    }
                    offset += 3;
                }
            }
        }

        wxAutoBufferedPaintDC dc(this);
        dc.DrawBitmap(wxBitmap(bar), 0, 0);
        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

        for (int i = 0; i < position_list.size(); i++)
        {
            int line_width = 1;
            if (i == current_selection)
            {
                line_width = 2;
            }
            gc->SetPen(wxPen(wxColor(0, 0, 0), line_width, wxPENSTYLE_SOLID));
            wxGraphicsPath path = gc->CreatePath();
            path.AddCircle(position_list.at(i) * bar_area.width / 255 + bar_area.x, bar_area.GetBottom() + r, r - 2 + line_width);
            gc->StrokePath(path);

            gc->SetPen(wxPen(wxColor(255, 255, 255), line_width, wxPENSTYLE_SOLID));
            gc->SetBrush(wxBrush(color_list.at(i).toWxColor(), wxBRUSHSTYLE_SOLID));
            path = gc->CreatePath();
            path.AddCircle(position_list.at(i) * bar_area.width / 255 + bar_area.x, bar_area.GetBottom() + r, r - 2);
            gc->DrawPath(path);
        }

        delete gc;
    }

    void onSize(wxSizeEvent &event)
    {
        wxSize size = event.GetSize();
        size.x = max(1, size.x);
        size.y = max(1, size.y);
        bar = wxImage(size);
        need_redraw_bar = true;
        Refresh();
        event.Skip();
    }

    void onMouse(wxMouseEvent &event)
    {
        int x = event.GetX();
        int y = event.GetY();
        int width = bar.GetWidth();
        int height = bar.GetHeight();

        // dragging_index = posToKnobIndex(x, y);

        if (event.LeftDown() && (posToKnobIndex(x, y) != -1))
        {
            dragging_index = posToKnobIndex(x, y);
            current_selection = dragging_index;
            CaptureMouse();
            clicking = true;
            Refresh();
        }

        if (event.Dragging())
        {
            clicking = false;
        }

        if (dragging_index != -1 && !clicking)
        {
            // is dragging
            moveKnob(dragging_index, (x - r) * 255 / (width - 2 * r));
            need_redraw_bar = true;
            Refresh();
            Update();
        }
        if (event.LeftUp() && dragging_index != -1)
        {
            ReleaseMouse();
            if (clicking)
            {
                // open color chooser
                popup->SetPosition(wxGetMousePosition());
                picker->setColor(color_list.at(dragging_index));
                wxRect screen_rect = wxRect(wxGetDisplaySize());
                wxRect window_rect = popup->GetRect();
                if (!screen_rect.Contains(window_rect))
                {
                    wxPoint offset = wxPoint(min(0, screen_rect.GetRight() - window_rect.GetRight()),
                                             min(0, screen_rect.GetBottom() - window_rect.GetBottom()));
                    popup->Move(window_rect.GetLeftTop() + offset);
                }

                popup->Popup();
                popup_index = dragging_index;
            }
            dragging_index = -1;
        }
    }

    void onColorPicker(wxCommandEvent &event)
    {
        Color *new_color = picker->getColor();
        int rgb[3];
        new_color->getRGB(rgb);
        (color_list.begin() + popup_index)->setRGB(rgb);
        delete new_color;
        need_redraw_bar = true;
        Refresh();
        Update();
        sendRampChangeEvent();
    }

    void moveKnob(int index, int value)
    {
        value = min(255, max(0, value));
        bool need_resort = false;
        if (index > 0)
        {
            if (value < position_list.at(index - 1))
            {
                // need resort
                need_resort = true;
            }
        }
        if (index < position_list.size() - 1)
        {
            if (value > position_list.at(index + 1))
            {
                // need resort
                need_resort = true;
            }
        }

        auto p = position_list.begin() + index;
        *p = value;

        if (need_resort)
        {
            RGBColor c = color_list.at(index);
            color_list.erase(color_list.begin() + index);
            int pos = position_list.at(index);
            position_list.erase(position_list.begin() + index);
            int new_index = insertKnob(pos, c);
            dragging_index = new_index;
            current_selection = dragging_index;
        }
        sendRampChangeEvent();
    }

    /** return index of the knob at the position.
     * -1 if in backgroudn
     */
    int posToKnobIndex(int x, int y)
    {
        for (int i = 0; i < position_list.size(); i++)
        {
            int pos_x = static_cast<double>(position_list.at(i)) / 255 * (bar.GetWidth() - 2 * r) + r;
            int pos_y = 2 * r + h;
            int dx = x - pos_x;
            int dy = y - pos_y;
            if (pow(dx, 2) + pow(dy, 2) <= pow(r, 2))
            {
                return i;
            }
        }
        return -1;
    }

    void sendRampChangeEvent(){
        wxCommandEvent* event = new wxCommandEvent(EVT_COLOR_RAMP_CHANGE, GetId());
        event->SetEventObject(this);
        wxQueueEvent(this->GetEventHandler(), event);
    }

    virtual wxSize DoGetBestSize() const override
    {
        return wxSize(200, 50);
    }
};

#endif // COLOR_RAMP_CTRL_H
