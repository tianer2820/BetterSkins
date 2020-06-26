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

/**
 * Layer Modifier should store their own data.
 * UI will be deleted when not on screen, the modifier should be able to remake them.
 * Modifier should bind the ui to their own handling functions.
 */
class LayerModifier
{
public:
    LayerModifier()
    {
    }
    LayerModifier(const LayerModifier &copy)
    {
        name = copy.name;
    }
    virtual ~LayerModifier()
    {
    }
    string getName()
    {
        return name;
    }
    virtual void render(wxImage &raw) = 0; // apply the modifier to the raw data
    virtual LayerModifier *copy() = 0;
    string name = "Modifier Base";

    void setVisible(bool visible = true)
    {
        this->visible = visible;
    }
    bool getVisible()
    {
        return visible;
    }
    /**
     * you need to override this method to create your controls on the panel.
     */
    virtual void makeUI(wxWindow *panel) = 0;

protected:
    bool visible = true;
    /**
     * you should send this event using a ctrl on your panel
     */
    wxCommandEvent *createLayerUpdateEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_LAYER_UPDATE);
        return event;
    }
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
    virtual void makeUI(wxWindow *panel) override
    {
        wxStaticText *label = new wxStaticText(panel, wxID_ANY, wxT("No configuration avaliable for this modifier"));
        wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
        box->Add(label, 1, wxALL | wxEXPAND, 5);
        panel->SetSizer(box);
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
        u_char *alpha_data = raw.GetAlpha();
        int length = raw.GetWidth() * raw.GetHeight();
        double blend = static_cast<double>(alpha_blend) / 255;
        for (int i = 0; i < length; i++)
        {
            alpha_data[i] *= blend;
        }
    }
    virtual LayerAlphaBlendModifier *copy()
    {
        return new LayerAlphaBlendModifier(*this);
    }
    virtual void makeUI(wxWindow *panel) override
    {
        alpha_slider = new wxSlider(panel, wxID_ANY, alpha_blend, 0, 255);
        alpha_slider->SetWindowStyleFlag(wxSL_VALUE_LABEL);
        wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
        wxStaticText *label = new wxStaticText(panel, wxID_ANY, wxString::FromUTF8("Alpha:"));
        box->Add(label, 0, wxALL | wxEXPAND, 5);
        box->Add(alpha_slider, 0, wxALL | wxEXPAND, 5);

        entry = new wxTextCtrl(panel, wxID_ANY, wxString(to_string(alpha_blend)));
        wxIntegerValidator<int> val;
        val.SetMin(0);
        val.SetMax(255);
        entry->SetValidator(val);
        box->Add(entry, 0, wxALL | wxEXPAND, 5);
        panel->SetSizer(box);
        panel->Bind(wxEVT_SLIDER, &LayerAlphaBlendModifier::onValueChange, this, alpha_slider->GetId());
        panel->Bind(wxEVT_TEXT, &LayerAlphaBlendModifier::onEntryChange, this, entry->GetId());
    }

protected:
    int alpha_blend = 255;
    wxSlider *alpha_slider;
    wxTextCtrl *entry;

    void onValueChange(wxCommandEvent &event)
    {
        wxCommandEvent *e = createLayerUpdateEvent();
        e->SetEventObject(alpha_slider);
        e->SetId(alpha_slider->GetId());
        wxQueueEvent(alpha_slider->GetEventHandler(), e);

        entry->ChangeValue(to_string(alpha_slider->GetValue()));
        alpha_blend = alpha_slider->GetValue();
    }
    void onEntryChange(wxCommandEvent &event)
    {
        wxCommandEvent *e = createLayerUpdateEvent();
        e->SetEventObject(entry);
        e->SetId(entry->GetId());
        wxQueueEvent(entry->GetEventHandler(), e);

        alpha_slider->SetValue(stoi(entry->GetValue().ToStdString()));
        alpha_blend = stoi(entry->GetValue().ToStdString());
    }
};

class LayerColorRampModifier : public LayerModifier
{
public:
    LayerColorRampModifier()
    {
        name = "Color Ramp";
    }
    virtual void render(wxImage &raw)
    {
        u_char *rgb = raw.GetData();
        int w = raw.GetWidth();
        int h = raw.GetHeight();

        for (int i = 0; i < w * h; i++)
        {
            int index = i * 3;
            int irgb[3];
            for (int c = 0; c < 3; c++)
            {
                irgb[c] = rgb[index + c];
            }
            int value = Color::RGB2GRAY(irgb);
            RGBColor color = getColorAt(value);
            color.getRGB(irgb);
            for (int c = 0; c < 3; c++)
            {
                rgb[index + c] = irgb[c];
            }
        }
    }
    virtual LayerModifier *copy()
    {
        return new LayerColorRampModifier(*this);
    }

    // used to create the control panel
    virtual void makeUI(wxWindow *panel)
    {
        ramp = new ColorRampBar(panel);
        if(is_init){
            doUpdate();
            is_init = false;
        } else{
            ramp->init(color_list, position_list);
        }

        panel->Bind(EVT_COLOR_RAMP_CHANGE, &LayerColorRampModifier::onRampChanged, this);

        wxButton *add_button = new wxButton(panel, wxID_ANY, "+");
        add_button->SetMaxSize(wxSize(30, 30));
        panel->Bind(wxEVT_BUTTON, &LayerColorRampModifier::onAdd, this, add_button->GetId());

        wxButton *delete_button = new wxButton(panel, wxID_ANY, "-");
        delete_button->SetMaxSize(wxSize(30, 30));
        panel->Bind(wxEVT_BUTTON, &LayerColorRampModifier::onDelete, this, delete_button->GetId());

        wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *box2 = new wxBoxSizer(wxHORIZONTAL);
        box->Add(ramp, 0, wxEXPAND);
        box->Add(box2, 0, wxEXPAND);
        box2->AddStretchSpacer();
        box2->Add(add_button, 0, wxALL, 2);
        box2->Add(delete_button, 0, wxALL, 2);

        panel->SetSizer(box);
    }

private:
    ColorRampBar *ramp = nullptr;
    vector<RGBColor> color_list;
    vector<int> position_list;

    bool is_init = true;

    void doUpdate(){
        wxCommandEvent* event = createLayerUpdateEvent();
        event->SetId(ramp->GetId());
        event->SetEventObject(ramp);
        wxQueueEvent(ramp->GetEventHandler(), event);

        color_list = ramp->getColorList();
        position_list = ramp->getPosList();
    }

    void onRampChanged(wxCommandEvent &event)
    {
        doUpdate();
    }
    void onAdd(wxCommandEvent &event)
    {
        ramp->add();
        doUpdate();
    }
    void onDelete(wxCommandEvent &event)
    {
        ramp->remove();
        doUpdate();
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
};

#endif // LAYER_MODIFIER_H
