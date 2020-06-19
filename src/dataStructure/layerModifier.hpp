#if !defined(LAYER_MODIFIER_H)
#define LAYER_MODIFIER_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/valnum.h>
#include <string>
#include "../customUI/layerModifierUIs/colorRampCtrl.hpp"

using namespace std;

wxDECLARE_EVENT(EVT_LAYER_UPDATE, wxCommandEvent);
wxDEFINE_EVENT(EVT_LAYER_UPDATE, wxCommandEvent);

class LayerModifier
{
public:
    LayerModifier()
    {
    }
    LayerModifier(const LayerModifier &copy)
    {
        name = copy.name;
        control_panel = NULL;
    }
    virtual ~LayerModifier()
    {
        // delete the controler panel
        if (control_panel != NULL)
        {
            control_panel->Destroy();
        }
    }
    wxPanel *getControlPanel(wxWindow *new_parent)
    {
        if (control_panel == NULL) // if the panel is not created
        {
            control_panel = new wxPanel(new_parent);
            createPanel();
        }
        else // if the panel is already created
        {
            control_panel->Reparent(new_parent);
        }
        return control_panel;
    }
    string getName()
    {
        return name;
    }
    virtual void render(wxImage &raw) = 0; // apply the modifier to the raw data
    virtual LayerModifier *copy() = 0;
    string name = "Modifier Base";

protected:
    void sendLayerUpdateEvent(){
        if(control_panel != NULL){
        wxCommandEvent* event = new wxCommandEvent(EVT_LAYER_UPDATE, control_panel->GetId());
        event->SetEventObject(control_panel);
        wxQueueEvent(control_panel->GetEventHandler(), event);
        }
    }
    virtual void createPanel() = 0; // used to create the control panel
    wxPanel *control_panel = NULL;
};

class LayerGrayModifier : public LayerModifier
{
public:
    LayerGrayModifier()
    {
        name = "Gray"; // set the display name
    }
    virtual void render(wxImage &raw)
    {
        u_char *raw_data = raw.GetData();
        int length = raw.GetWidth() * raw.GetHeight();
        for (int i = 0; i < length; i++)
        {
            int rgb[3];
            for (int c = 0; c < 3; c++)
            {
                rgb[c] = raw_data[i * 3 + c];
            }
            u_char gray = Color::RGB2GRAY(rgb);
            for (int c = 0; c < 3; c++)
            {
                raw_data[i * 3 + c] = gray;
            }
        }
    }
    virtual LayerGrayModifier *copy()
    {
        return new LayerGrayModifier(*this);
    }

protected:
    virtual void createPanel()
    {
        wxStaticText *label = new wxStaticText(control_panel, wxID_ANY, wxT("No configuration avaliable for this modifier"));
        wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
        box->Add(label, 1, wxALL | wxEXPAND, 5);
        control_panel->SetSizer(box);
    }
};

class LayerAlphaBlendModifier : public LayerModifier
{
public:
    LayerAlphaBlendModifier()
    {
        name = "Alpha Blend"; // set the display name
    }
    virtual void render(wxImage &raw)
    {
        u_char* alpha_data = raw.GetAlpha();
        int length = raw.GetWidth() * raw.GetHeight();
        double blend = static_cast<double>(alpha_slider->GetValue()) / 255;
        for (int i = 0; i < length; i++)
        {
            alpha_data[i] *= blend;
        }
    }
    virtual LayerAlphaBlendModifier *copy()
    {
        return new LayerAlphaBlendModifier(*this);
    }
protected:
    wxSlider* alpha_slider;
    wxTextCtrl* entry;
    virtual void createPanel()
    {
        alpha_slider = new wxSlider(control_panel, wxID_ANY, 255, 0, 255);
        alpha_slider->SetWindowStyleFlag(wxSL_VALUE_LABEL);
        wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
        wxStaticText* label = new wxStaticText(control_panel, wxID_ANY, wxString::FromUTF8("Alpha:"));
        box->Add(label, 0, wxALL | wxEXPAND, 5);
        box->Add(alpha_slider, 0, wxALL | wxEXPAND, 5);

        entry = new wxTextCtrl(control_panel, wxID_ANY, wxString::FromUTF8("255"));
        wxIntegerValidator<int> val;
        val.SetMin(0);
        val.SetMax(255);
        entry->SetValidator(val);
        box->Add(entry, 0, wxALL | wxEXPAND, 5);
        control_panel->SetSizer(box);
        control_panel->Bind(wxEVT_SLIDER, &LayerAlphaBlendModifier::onValueChange, this, alpha_slider->GetId());
        control_panel->Bind(wxEVT_TEXT, &LayerAlphaBlendModifier::onEntryChange, this, entry->GetId());
    }
    void onValueChange(wxCommandEvent& event){
        sendLayerUpdateEvent();
        entry->ChangeValue(to_string(alpha_slider->GetValue()));
    }
    void onEntryChange(wxCommandEvent& event){
        sendLayerUpdateEvent();
        alpha_slider->SetValue(stoi(entry->GetValue().ToStdString()));
    }
};

class LayerColorRampModifier : public LayerModifier{
    public:
    LayerColorRampModifier(){
        name = "Color Ramp";
    }
    virtual void render(wxImage &raw){
        u_char* rgb = raw.GetData();
        int w = raw.GetWidth();
        int h = raw.GetHeight();

        for(int i = 0; i < w * h; i++){
            int index = i * 3;
            int irgb[3];
            for (int c = 0; c < 3; c++)
            {
                irgb[c] = rgb[index + c];
            }
            int value = Color::RGB2GRAY(irgb);
            RGBColor color = ramp->mapColor(value);
            color.getRGB(irgb);
            for (int c = 0; c < 3; c++)
            {
                rgb[index + c] = irgb[c];
            }
        }
    }
    virtual LayerModifier *copy(){
        return new LayerColorRampModifier(*this);
    }
    private:
    ColorRampBar* ramp = nullptr;

    void onRampChanged(wxCommandEvent& event){
        sendLayerUpdateEvent();
    }

    void onAdd(wxCommandEvent& event){
        ramp->add();
        sendLayerUpdateEvent();
    }
    void onDelete(wxCommandEvent& event){
        ramp->remove();
        sendLayerUpdateEvent();
    }

    // used to create the control panel
    virtual void createPanel(){
        ramp = new ColorRampBar(this->control_panel);
        control_panel->Bind(EVT_COLOR_RAMP_CHANGE, &LayerColorRampModifier::onRampChanged, this);

        wxButton* add_button = new wxButton(this->control_panel, wxID_ANY, "+");
        add_button->SetMaxSize(wxSize(30, 30));
        control_panel->Bind(wxEVT_BUTTON, &LayerColorRampModifier::onAdd, this, add_button->GetId());
        
        wxButton* delete_button = new wxButton(this->control_panel, wxID_ANY, "-");
        delete_button->SetMaxSize(wxSize(30, 30));
        control_panel->Bind(wxEVT_BUTTON, &LayerColorRampModifier::onDelete, this, delete_button->GetId());

        wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* box2 = new wxBoxSizer(wxHORIZONTAL);
        box->Add(ramp, 0, wxEXPAND);
        box->Add(box2, 0, wxEXPAND);
        box2->AddStretchSpacer();
        box2->Add(add_button, 0, wxALL, 2);
        box2->Add(delete_button, 0, wxALL, 2);

        control_panel->SetSizer(box);
    }
};


#endif // LAYER_MODIFIER_H
