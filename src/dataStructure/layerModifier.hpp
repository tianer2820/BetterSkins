#if !defined(LAYER_MODIFIER_H)
#define LAYER_MODIFIER_H

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

wxDECLARE_EVENT(EVT_LAYER_UPDATE, wxCommandEvent);

/**
 * Layer Modifier should store their own data.
 * UI will be deleted when not on screen, the modifier should be able to remake them.
 * Modifier should bind the ui to their own handling functions.
 */
class LayerModifier
{
public:
    LayerModifier();
    LayerModifier(const LayerModifier &copy);
    virtual ~LayerModifier();
    string getName();
    virtual void render(wxImage &raw) = 0; // apply the modifier to the raw data
    virtual LayerModifier *copy() = 0;
    string name = "Modifier Base";

    void setVisible(bool visible = true);
    bool getVisible();
    /**
     * you need to override this method to create your controls on the panel.
     */
    virtual void makeUI(wxWindow *panel) = 0;
    virtual json toJson() = 0;
    virtual bool loadJson(json j) = 0;
    bool isModified();
    void setModified(bool v);

protected:
    bool visible = true;
    bool modified = true;

    /**
     * you should send this event using a ctrl on your panel
     */
    wxCommandEvent *createLayerUpdateEvent();
};

class LayerGrayModifier : public LayerModifier
{
public:
    LayerGrayModifier();
    virtual void render(wxImage &raw);

    virtual json toJson();
    virtual bool loadJson(json j);

    virtual LayerGrayModifier *copy();

protected:
    virtual void makeUI(wxWindow *panel) override;
};

class LayerAlphaBlendModifier : public LayerModifier
{
public:
    LayerAlphaBlendModifier();
    virtual void render(wxImage &raw);
    virtual LayerAlphaBlendModifier *copy();
    virtual void makeUI(wxWindow *panel) override;

    virtual json toJson();
    virtual bool loadJson(json j);

protected:
    int alpha_blend = 255;
    wxSlider *alpha_slider;
    wxTextCtrl *entry;

    void onValueChange(wxCommandEvent &event);
    void onEntryChange(wxCommandEvent &event);
};

class LayerColorRampModifier : public LayerModifier
{
public:
    LayerColorRampModifier();
    virtual void render(wxImage &raw);
    virtual LayerModifier *copy();

    // used to create the control panel
    virtual void makeUI(wxWindow *panel);

    virtual json toJson();
    virtual bool loadJson(json j);

private:
    ColorRampBar *ramp = nullptr;
    vector<RGBColor> color_list;
    vector<int> position_list;

    bool is_init = true;

    void doUpdate();

    void onRampChanged(wxCommandEvent &event);
    void onAdd(wxCommandEvent &event);
    void onDelete(wxCommandEvent &event);

    RGBColor getColorAt(int pos);
    RGBColor mixRGB(RGBColor c0, RGBColor c1, double percent);
};

#endif // LAYER_MODIFIER_H
