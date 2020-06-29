
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/notebook.h>

#include "../../color/color.h"
#include "colorCircle.h"
#include "colorSquare.h"
#include "rgbPanel.h"
#include "hsvPanel.h"
#include "hexPanel.hpp"
#include "colorShower.hpp"

#include "advColorPicker.hpp"
using namespace std;

AdvColorPicker::AdvColorPicker(wxWindow *parent, wxWindowID id) : wxPanel(parent, id)
{
    wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);

    wxNotebook *upper_book = new wxNotebook(this, wxID_ANY);

    ColorCircle *circle = new ColorCircle(upper_book);
    Bind(EVT_COLOR_PICKER_CHANGE, &AdvColorPicker::onColorChange, this);
    color_picker_list[0] = circle;
    upper_book->AddPage(circle, wxString::FromUTF8("Circle"));

    ColorSquare *square = new ColorSquare(upper_book);
    Bind(EVT_COLOR_PICKER_CHANGE, &AdvColorPicker::onColorChange, this);
    color_picker_list[1] = square;
    upper_book->AddPage(square, wxString::FromUTF8("Square"));

    box->Add(upper_book, 1, wxEXPAND | wxALL, 5);

    color_shower = new ColorShower(this);
    box->Add(color_shower, 0, wxEXPAND | wxALL, 5);

    wxNotebook *lower_book = new wxNotebook(this, wxID_ANY);

    HSVBarPanel *HSVPage = new HSVBarPanel(lower_book);
    RGBBarPanel *RGBPage = new RGBBarPanel(lower_book);
    color_picker_list[2] = RGBPage;
    color_picker_list[3] = HSVPage;
    Bind(EVT_COLOR_PICKER_CHANGE, &AdvColorPicker::onColorChange, this);

    lower_book->AddPage(RGBPage, wxString::FromUTF8("RGB"));
    lower_book->AddPage(HSVPage, wxString::FromUTF8("HSV"));
    box->Add(lower_book, 0, wxEXPAND | wxALL, 5);

    HEXPanel *HEXPage = new HEXPanel(this);
    color_picker_list[4] = HEXPage;
    box->Add(HEXPage, 0, wxEXPAND | wxALL, 5);

    this->SetSizer(box);
}

// you have to delete the returned object manually!
Color *AdvColorPicker::getColor()
{
    return color_picker_list[1]->getColor(); // use RGB for speed
}
void AdvColorPicker::setColor(Color &color)
{
    for (int i = 0; i < num_pickers; i += 1)
    {
        color_picker_list[i]->setColor(color);
    }
    int rgb[3];
    color.getRGB(rgb);
    color_shower->setColor(rgb, color.getAlpha());
    color_shower->Update();
}

void AdvColorPicker::onColorChange(wxCommandEvent &event)
{
    ColorPicker *changed_picker = dynamic_cast<ColorPicker *>(event.GetEventObject());
    int index = -1;
    for (int i = 0; i < num_pickers; i += 1)
    {
        if (changed_picker == color_picker_list[i])
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        throw "index error";
    }
    Color *new_color = changed_picker->getColor();
    for (int i = 0; i < num_pickers; i += 1)
    {
        if (i == index)
        {
            continue;
        }
        color_picker_list[i]->setColor(*new_color);
    }
    int rgb[3];
    new_color->getRGB(rgb);
    color_shower->setColor(rgb, new_color->getAlpha());
    color_shower->Update();
    delete new_color;
    sendColorChangeEvent();
}

void AdvColorPicker::sendColorChangeEvent()
{
    wxCommandEvent *event = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, GetId());
    event->SetEventObject(this);
    wxWindow *parent = GetParent();
    if (parent != NULL)
    {
        wxQueueEvent(parent->GetEventHandler(), event);
    }
}

wxSize AdvColorPicker::DoGetBestSize() const
{
    return wxSize(50, 50);
}
