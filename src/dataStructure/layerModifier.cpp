#include "layerModifier.hpp"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/valnum.h>
#include <string>
#include <json.hpp>
using json = nlohmann::json;
#include "../customUI/layerModifierUIs/colorRampCtrl.hpp"

using namespace std;

wxDEFINE_EVENT(EVT_LAYER_UPDATE, wxCommandEvent);

LayerModifier::LayerModifier()
{
}
LayerModifier::LayerModifier(const LayerModifier &copy)
{
    name = copy.name;
}
LayerModifier::~LayerModifier()
{
}
string LayerModifier::getName()
{
    return name;
}

void LayerModifier::setVisible(bool visible)
{
    this->visible = visible;
}
bool LayerModifier::getVisible()
{
    return visible;
}

bool LayerModifier::isModified()
{
    return modified;
}
void LayerModifier::setModified(bool v)
{
    modified = v;
}
wxCommandEvent *LayerModifier::createLayerUpdateEvent()
{
    wxCommandEvent *event = new wxCommandEvent(EVT_LAYER_UPDATE);
    return event;
}

LayerGrayModifier::LayerGrayModifier()
{
    name = "Gray"; // set the display name
}
void LayerGrayModifier::render(wxImage &raw)
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

json LayerGrayModifier::toJson()
{
    json j;
    j["modifier_name"] = name;
    return j;
}
bool LayerGrayModifier::loadJson(json j)
{
    modified = false;
    return true;
}

LayerGrayModifier *LayerGrayModifier::copy()
{
    return new LayerGrayModifier(*this);
}

void LayerGrayModifier::makeUI(wxWindow *panel)
{
    wxStaticText *label = new wxStaticText(panel, wxID_ANY, wxT("No configuration avaliable for this modifier"));
    wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
    box->Add(label, 1, wxALL | wxEXPAND, 5);
    panel->SetSizer(box);
}

LayerAlphaBlendModifier::LayerAlphaBlendModifier()
{
    name = "Alpha Blend"; // set the display name
}
void LayerAlphaBlendModifier::render(wxImage &raw)
{
    u_char *alpha_data = raw.GetAlpha();
    int length = raw.GetWidth() * raw.GetHeight();
    double blend = static_cast<double>(alpha_blend) / 255;
    for (int i = 0; i < length; i++)
    {
        alpha_data[i] *= blend;
    }
}
LayerAlphaBlendModifier *LayerAlphaBlendModifier::copy()
{
    LayerAlphaBlendModifier *c = new LayerAlphaBlendModifier(*this);
    c->alpha_slider = nullptr;
    c->entry = nullptr;
    return c;
}
void LayerAlphaBlendModifier::makeUI(wxWindow *panel)
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

json LayerAlphaBlendModifier::toJson()
{
    json j;
    j["modifier_name"] = name;
    j["alpha"] = alpha_blend;
    return j;
}
bool LayerAlphaBlendModifier::loadJson(json j)
{
    if (j.contains("alpha") && j["alpha"].is_number_integer())
    {
        alpha_blend = j["alpha"];
    }
    else
    {
        return false;
    }
    modified = false;
    return true;
}

void LayerAlphaBlendModifier::onValueChange(wxCommandEvent &event)
{
    wxCommandEvent *e = createLayerUpdateEvent();
    e->SetEventObject(alpha_slider);
    e->SetId(alpha_slider->GetId());
    wxQueueEvent(alpha_slider->GetEventHandler(), e);

    entry->ChangeValue(to_string(alpha_slider->GetValue()));
    alpha_blend = alpha_slider->GetValue();
    modified = true;
}
void LayerAlphaBlendModifier::onEntryChange(wxCommandEvent &event)
{
    wxCommandEvent *e = createLayerUpdateEvent();
    e->SetEventObject(entry);
    e->SetId(entry->GetId());
    wxQueueEvent(entry->GetEventHandler(), e);

    alpha_slider->SetValue(stoi(entry->GetValue().ToStdString()));
    alpha_blend = stoi(entry->GetValue().ToStdString());
    modified = true;
}

LayerColorRampModifier::LayerColorRampModifier()
{
    name = "Color Ramp";
}
void LayerColorRampModifier::render(wxImage &raw)
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
LayerModifier *LayerColorRampModifier::copy()
{
    LayerColorRampModifier *c = new LayerColorRampModifier(*this);
    c->ramp = nullptr;
    return c;
}

// used to create the control panel
void LayerColorRampModifier::makeUI(wxWindow *panel)
{
    ramp = new ColorRampBar(panel);
    if (is_init)
    {
        doUpdate();
        is_init = false;
    }
    else
    {
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

json LayerColorRampModifier::toJson()
{
    json j;
    j["modifier_name"] = name;
    j["color_list"] = json::array();
    j["position_list"] = json::array();
    int len = color_list.size();
    for (int i = 0; i < len; i++)
    {
        RGBColor color = color_list.at(i);
        int pos = position_list.at(i);
        int rgb[3];
        color.getRGB(rgb);
        json json_color;
        for (int c = 0; c < 3; c++)
        {
            json_color.push_back(rgb[c]);
        }
        j["color_list"].push_back(json_color);
        j["position_list"].push_back(pos);
    }

    return j;
}
bool LayerColorRampModifier::loadJson(json j)
{
    if (!j.is_object())
    {
        return false;
    }

    if (j.contains("modifier_name"))
    {
        name = j["modifier_name"].get<string>();
    }
    else
    {
        return false;
    }

    if (j.contains("color_list") && j.contains("position_list"))
    {
        json colors = j["color_list"];
        json positions = j["position_list"];
        int size = colors.size();
        for (int i = 0; i < size; i++)
        {
            json c = colors[i];
            int pos = positions[i];
            position_list.push_back(pos);
            color_list.push_back(RGBColor(c[0], c[1], c[2]));
        }
    }
    else
    {
        return false;
    }
    is_init = false;
    modified = false;
    return true;
}

void LayerColorRampModifier::doUpdate()
{
    wxCommandEvent *event = createLayerUpdateEvent();
    event->SetId(ramp->GetId());
    event->SetEventObject(ramp);
    wxQueueEvent(ramp->GetEventHandler(), event);

    color_list = ramp->getColorList();
    position_list = ramp->getPosList();
}

void LayerColorRampModifier::onRampChanged(wxCommandEvent &event)
{
    doUpdate();
    modified = true;
}
void LayerColorRampModifier::onAdd(wxCommandEvent &event)
{
    ramp->add();
    doUpdate();
    modified = true;
}
void LayerColorRampModifier::onDelete(wxCommandEvent &event)
{
    ramp->remove();
    doUpdate();
    modified = true;
}

RGBColor LayerColorRampModifier::getColorAt(int pos)
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
RGBColor LayerColorRampModifier::mixRGB(RGBColor c0, RGBColor c1, double percent)
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
