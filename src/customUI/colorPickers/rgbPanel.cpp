
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/spinctrl.h>

#include "../../color/color.h"
#include "colorBars.h"
#include "colorPicker.h"
#include "rgbPanel.h"
using namespace std;

RGBBarPanel::RGBBarPanel(wxWindow *parent, wxWindowID id) : wxWindow(parent, id)
{
    wxFlexGridSizer *grid = new wxFlexGridSizer(2);
    grid->AddGrowableCol(0, 1);
    for (int i = 0; i < 3; i += 1)
    {
        bar_list[i] = new RGBBar(this, wxID_ANY, i);
        spin_list[i] = new wxSpinCtrl(this, wxID_ANY);
        spin_list[i]->SetMin(0);
        spin_list[i]->SetMax(255);

        grid->Add(bar_list[i], 1, wxEXPAND | wxALL, 2);
        grid->Add(spin_list[i], 0, wxEXPAND | wxALL, 2);

        Bind(EVT_COLOR_BAR_CHANGE, &RGBBarPanel::onBarChange, this, bar_list[i]->GetId());
        Bind(wxEVT_SPINCTRL, &RGBBarPanel::onSpinChange, this, spin_list[i]->GetId());
    }
    this->SetSizer(grid);
}
Color *RGBBarPanel::getColor()
{
    Color *out = new RGBColor();
    int rgb[3];
    for (int i = 0; i < 3; i += 1)
    {
        rgb[i] = bar_list[i]->getValue();
    }
    out->setRGB(rgb);
    return out;
}
void RGBBarPanel::setColor(Color &color)
{
    int rgb[3];
    color.getRGB(rgb);
    for (int i = 0; i < 3; i += 1)
    {
        bar_list[i]->setColor(rgb);
        spin_list[i]->SetValue(rgb[i]);
    }
    Update();
}

void RGBBarPanel::onBarChange(wxCommandEvent &event)
{
    // update spins
    ColorBar *bar = (ColorBar *)event.GetEventObject();
    int index = 0;
    int id = bar->GetId();
    for (int i = 0; i < 3; i += 1)
    {
        if (bar_list[i]->GetId() == id)
        {
            index = i;
            break;
        }
    }
    int value = bar_list[index]->getValue();
    spin_list[index]->SetValue(value);
    for (int i = 0; i < 3; i += 1)
    {
        if (i == index)
        {
            continue;
        }
        bar_list[i]->setChannel(index, value);
    }
    sendColorChangeEvent();
    Update();
}
void RGBBarPanel::onSpinChange(wxSpinEvent &event)
{
    // update bars
    wxSpinCtrl *spin = (wxSpinCtrl *)event.GetEventObject();
    int index = 0;
    int id = spin->GetId();
    for (int i = 0; i < 3; i += 1)
    {
        if (spin_list[i]->GetId() == id)
        {
            index = i;
            break;
        }
    }
    int value = spin_list[index]->GetValue();
    for (int i = 0; i < 3; i += 1)
    {
        bar_list[i]->setChannel(index, value);
    }
    sendColorChangeEvent();
}
void RGBBarPanel::sendColorChangeEvent()
{
    wxCommandEvent *event = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, GetId());
    event->SetEventObject(this);
    wxQueueEvent(GetEventHandler(), event);
}
