#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/spinctrl.h>

#include "../../color/color.h"
#include "colorBars.h"
#include "colorPicker.h"
#include "hsvPanel.h"
using namespace std;

HSVBarPanel::HSVBarPanel(wxWindow *parent, wxWindowID id) : wxWindow(parent, id)
{
    wxFlexGridSizer *grid = new wxFlexGridSizer(2);
    grid->AddGrowableCol(0, 1);
    for (int i = 0; i < 3; i += 1)
    {
        barList[i] = new HSVBar(this, wxID_ANY, i);
        spinList[i] = new wxSpinCtrl(this, wxID_ANY);
        if (i == 0)
        {
            spinList[i]->SetRange(0, 359);
            int c[3] = {0, 255, 255};
            barList[i]->setColor(c);
        }
        else
        {
            spinList[i]->SetRange(0, 255);
        }

        grid->Add(barList[i], 1, wxEXPAND | wxALL, 2);
        grid->Add(spinList[i], 0, wxEXPAND | wxALL, 2);
        Bind(EVT_COLOR_BAR_CHANGE, &HSVBarPanel::onBarChange, this, barList[i]->GetId());
        Bind(wxEVT_SPINCTRL, &HSVBarPanel::onSpinChange, this, spinList[i]->GetId());
    }
    this->SetSizer(grid);
}
Color *HSVBarPanel::getColor()
{
    Color *out = new HSVColor();
    int hsv[3];
    for (int i = 0; i < 3; i += 1)
    {
        hsv[i] = barList[i]->getValue();
    }
    out->setHSV(hsv);
    return out;
}
void HSVBarPanel::setColor(Color &color)
{
    int hsv[3];
    color.getHSV(hsv);
    barList[0]->setValue(hsv[0]);
    spinList[0]->SetValue(hsv[0]);
    for (int i = 1; i < 3; i += 1)
    {
        barList[i]->setColor(hsv);
        spinList[i]->SetValue(hsv[i]);
    }
    Update();
}
void HSVBarPanel::onBarChange(wxCommandEvent &event)
{
    // update spins
    ColorBar *bar = (ColorBar *)event.GetEventObject();
    int index = 0;
    int id = bar->GetId();
    for (int i = 0; i < 3; i += 1)
    {
        if (barList[i]->GetId() == id)
        {
            index = i;
            break;
        }
    }
    int value = barList[index]->getValue();
    spinList[index]->SetValue(value);
    for (int i = 1; i < 3; i += 1)
    {
        if (i == index)
        {
            continue;
        }
        barList[i]->setChannel(index, value);
    }
    sendColorChangeEvent();
    Update();
}
void HSVBarPanel::onSpinChange(wxSpinEvent &event)
{
    // update bars
    wxSpinCtrl *spin = (wxSpinCtrl *)event.GetEventObject();
    int index = 0;
    int id = spin->GetId();
    for (int i = 0; i < 3; i += 1)
    {
        if (spinList[i]->GetId() == id)
        {
            index = i;
            break;
        }
    }
    int value = spinList[index]->GetValue();
    if (index == 0)
    {
        barList[0]->setChannel(index, value);
    }
    for (int i = 1; i < 3; i += 1)
    {
        barList[i]->setChannel(index, value);
    }
    sendColorChangeEvent();
}
void HSVBarPanel::sendColorChangeEvent()
{
    wxCommandEvent *event = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, GetId());
    event->SetEventObject(this);
    wxQueueEvent(GetEventHandler(), event);
}
