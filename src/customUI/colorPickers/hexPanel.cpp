#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../color/color.h"
#include "colorPicker.h"
#include "hexPanel.hpp"
using namespace std;


    HEXPanel::HEXPanel(wxWindow *parent, wxWindowID id) : wxPanel(parent, id)
    {
        wxBoxSizer* box = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText* label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("HEX"));
        box->Add(label, 0, wxRIGHT | wxEXPAND, 5);
        entry = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        box->Add(entry, 1, wxALL | wxEXPAND, 0);
        this->SetSizer(box);

        Bind(wxEVT_TEXT_ENTER, &HEXPanel::onTextChange, this);
    }
     Color* HEXPanel::getColor()
    {
        wxString str = entry->GetValue();
        Color* out = new RGBColor();
        if(str.length() != 6){
            return out;
        }
        out->setHEX(str.ToStdString());
        return out;
    }
     void HEXPanel::setColor(Color &color)
    {
        string str = color.getHEX();
        entry->SetValue(wxString::FromUTF8(str).Upper());
        Update();
    }
    void HEXPanel::onTextChange(wxCommandEvent& event){
        wxCommandEvent *e = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, GetId());
        e->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), e);
    }
