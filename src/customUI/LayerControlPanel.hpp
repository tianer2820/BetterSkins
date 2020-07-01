#if !defined(LAYER_CONTROL_PANEL_H)
#define LAYER_CONTROL_PANEL_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listbook.h>

#include "dataStructure/layer.hpp"
#include "dataStructure/layerModifier.hpp"

wxDECLARE_EVENT(EVT_LAYER_RENAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_LAYER_RENAME, wxCommandEvent);

/**
 * To display and change the modifier list or name of a layer.
 * Call load to load a layer, and clear to unload.
 * Changes in layers are automatically made when user click the buttons.
 * 
 * This class will send two events:
 * EVT_LAYER_RENAME: when the name text is changed
 * EVT_LAYER_UPDATE: when the modifier is changed
 */
class LayerControlPanel : public wxPanel
{
public:
    LayerControlPanel(wxWindow *parent, wxWindowID id = wxID_ANY) : wxPanel(parent, id)
    {
        // layer name display
        wxBoxSizer *main_box = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *name_box = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Layer:"));
        name_box->Add(label, 0, wxALL | wxEXPAND, 5);
        name_ctrl = new wxTextCtrl(this, wxID_ANY);
        Bind(wxEVT_TEXT, &LayerControlPanel::onRename, this, name_ctrl->GetId());
        name_box->Add(name_ctrl, 1, wxALL | wxEXPAND, 5);
        main_box->Add(name_box, 0, wxEXPAND);

        // buttons
        wxImage icon;
        wxString icon_path = _T("./resources/icons/");
        wxBoxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
        //add
        icon.LoadFile(icon_path + _T("add.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_add = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_add->SetToolTip("Add a modifier to this layer");
        //delete
        icon.LoadFile(icon_path + _T("delete.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_delete = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_delete->SetToolTip("Delete the selected modifier");
        //up
        icon.LoadFile(icon_path + _T("up.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_up = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_up->SetToolTip("Move this modifier up the stack");
        //down
        icon.LoadFile(icon_path + _T("down.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_down = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_down->SetToolTip("Move this modifier down the stack");

        but_add->SetMinSize(wxSize(20, 20));
        but_delete->SetMinSize(wxSize(20, 20));
        but_up->SetMinSize(wxSize(20, 20));
        but_down->SetMinSize(wxSize(20, 20));
        Bind(wxEVT_BUTTON, &LayerControlPanel::onAdd, this, but_add->GetId());
        Bind(wxEVT_BUTTON, &LayerControlPanel::onDelete, this, but_delete->GetId());
        Bind(wxEVT_BUTTON, &LayerControlPanel::onMoveUp, this, but_up->GetId());
        Bind(wxEVT_BUTTON, &LayerControlPanel::onMoveDown, this, but_down->GetId());
        button_box->Add(but_add, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_delete, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_up, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_down, 1, wxALL | wxEXPAND, 2);
        main_box->Add(button_box, 0, wxEXPAND);
        // listbook
        list_book = new wxListbook(this, wxID_ANY);
        list_book->SetWindowStyle(wxLB_LEFT);
        main_box->Add(list_book, 1, wxALL | wxEXPAND, 5);

        SetSizer(main_box);
    }
    /**
     * call "clear" to unload the layer
     */
    void loadLayer(Layer *layer)
    {
        // remove all pages
        int len = list_book->GetPageCount();
        for (int i = 0; i < len; i++)
        {
            wxWindow *page = list_book->GetPage(0);
            wxWindowListNode *node = list_book->GetChildren().Find(page);
            if (node != NULL)
            { // only hide if the panel is not deleted
                page->Show(false);
            }
            list_book->DeletePage(0);
        }
        // set active layer
        active_layer = layer;
        // add new pages
        vector<LayerModifier *> modifier_list = layer->getModifierList();
        for (auto i = modifier_list.begin(); i != modifier_list.end(); i++)
        {
            LayerModifier *modifier = *i;
            prepareModifierUI(modifier);
        }
        // load name
        name_ctrl->ChangeValue(wxString::FromUTF8(active_layer->getName()));
    }
    /** 
     * unload the current layer
     */
    void clear()
    {
        // remove all pages
        int len = list_book->GetPageCount();
        for (int i = 0; i < len; i++)
        {
            list_book->DeletePage(0);
        }
        // set layer to NULL
        active_layer = NULL;
        // clear string
        name_ctrl->ChangeValue(_T(""));
    }

protected:
    wxTextCtrl *name_ctrl;
    wxListbook *list_book;
    Layer *active_layer = NULL;

    void prepareModifierUI(LayerModifier *modifier)
    {
        wxPanel *panel = new wxPanel(list_book);
        wxPanel *control_panel = new wxPanel(panel);
        modifier->makeUI(control_panel);
        wxCheckBox *visable_box = new wxCheckBox(panel, wxID_ANY, _T("Visable"));
        wxButton *apply_but = new wxButton(panel, wxID_ANY, _T("Apply"));
        wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);

        box->Add(apply_but, 0, wxALL | wxEXPAND, 2);
        box->Add(visable_box, 0, wxALL | wxEXPAND, 2);
        box->Add(control_panel, 1, wxALL | wxEXPAND, 2);
        panel->SetSizer(box);
        visable_box->SetValue(modifier->getVisible());
        panel->Bind(wxEVT_CHECKBOX, &LayerControlPanel::onToggleVisable, this, visable_box->GetId());
        panel->Bind(wxEVT_BUTTON, &LayerControlPanel::onApply, this, apply_but->GetId());

        list_book->AddPage(panel, wxString::FromUTF8(modifier->getName()));
        panel->Show();
        box->ShowItems(true);
    }

    void onToggleVisable(wxCommandEvent &event)
    {
        if (active_layer == nullptr)
        {
            return; // weird thing happend... but return anyway.
        }
        int modifier_index = list_book->GetSelection();
        if (modifier_index == wxNOT_FOUND)
        {
            return; // weird thing happend again..
        }
        LayerModifier *modifier = active_layer->getModifierList().at(modifier_index);
        wxCheckBox *box = dynamic_cast<wxCheckBox *>(event.GetEventObject());
        if (box == nullptr)
        {
            return;
        }
        bool value = box->GetValue();
        modifier->setVisible(value);
        sendUpdateEvent();
    }
    void onApply(wxCommandEvent & event){
        if (active_layer == nullptr)
        {
            return; // weird thing happend... return anyway.
        }
        int modifier_index = list_book->GetSelection();
        if (modifier_index == wxNOT_FOUND)
        {
            return; // weird thing happend again..
        }
        LayerModifier *modifier = active_layer->getModifierList().at(modifier_index);
        
        modifier->render(*(active_layer->getImage()));
        
        active_layer->deleteModifier(modifier_index);
        list_book->DeletePage(modifier_index);
        sendUpdateEvent();
    }
    void onRename(wxCommandEvent &event)
    {
        if (active_layer == nullptr)
        {
            return;
        }
        if (name_ctrl->GetValue() != "")
        {
            active_layer->setName(name_ctrl->GetValue().ToStdString());
            sendRenameEvent();
        }
    }
    void onAdd(wxCommandEvent &event)
    {
        if (active_layer == NULL)
        {
            return;
        }
        wxArrayString choices;
        choices.Add(wxString::FromUTF8("Gray Scale"));
        choices.Add(wxString::FromUTF8("Alpha Blend"));
        choices.Add(wxString::FromUTF8("Color Ramp"));

        int choice = wxGetSingleChoiceIndex(wxString::FromUTF8("Choose the modifier:"), wxString::FromUTF8("New Modifier"), choices);
        LayerModifier *modifier;
        switch (choice)
        {
        case -1:
            // user cancled
            return;
            break;
        case 0:
            // gray modifier
            modifier = new LayerGrayModifier();
            break;
        case 1:
            // Alpha blend
            modifier = new LayerAlphaBlendModifier();
            break;
        case 2:
            // color ramp
            modifier = new LayerColorRampModifier();
            break;
        default:
            return;
            break;
        }
        active_layer->addModifier(modifier);
        prepareModifierUI(modifier);
        list_book->SetSelection(list_book->GetPageCount() - 1);
        sendUpdateEvent();
    }
    void onDelete(wxCommandEvent &event)
    {
        int index = list_book->GetSelection();
        if (active_layer == NULL || index == -1)
        {
            return;
        }
        list_book->DeletePage(index);
        active_layer->deleteModifier(index);
        sendUpdateEvent();
    }
    void onMoveUp(wxCommandEvent &event)
    {
        int index = list_book->GetSelection();
        if (active_layer == NULL || index == 0 || index == -1)
        {
            return;
        }
        active_layer->moveModefier(index, index - 1);
        wxWindow *page = list_book->GetPage(index);
        wxString name = list_book->GetPageText(index);
        list_book->RemovePage(index);
        list_book->InsertPage(index - 1, page, name);
        list_book->SetSelection(index - 1);
        sendUpdateEvent();
    }
    void onMoveDown(wxCommandEvent &event)
    {
        int index = list_book->GetSelection();
        if (active_layer == NULL || index == list_book->GetPageCount() - 1 || index == -1)
        {
            return;
        }
        active_layer->moveModefier(index, index + 1);
        wxWindow *page = list_book->GetPage(index);
        wxString name = list_book->GetPageText(index);
        list_book->RemovePage(index);
        list_book->InsertPage(index + 1, page, name);
        list_book->SetSelection(index + 1);
        sendUpdateEvent();
    }
    void sendRenameEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_LAYER_RENAME, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
    void sendUpdateEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_LAYER_UPDATE, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
};

#endif // LAYER_CONTROL_PANEL_H
