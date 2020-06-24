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
        wxBoxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
        wxButton *but_add = new wxButton(this, wxID_ANY, wxString::FromUTF8("+"));
        wxButton *but_delete = new wxButton(this, wxID_ANY, wxString::FromUTF8("-"));
        wxButton *but_up = new wxButton(this, wxID_ANY, wxString::FromUTF8("UP"));
        wxButton *but_down = new wxButton(this, wxID_ANY, wxString::FromUTF8("DOWN"));
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
            list_book->RemovePage(0);
        }
        // set active layer
        active_layer = layer;
        // add new pages
        vector<LayerModifier *> layers = layer->getModifierList();
        for (auto i = layers.begin(); i != layers.end(); i++)
        {
            LayerModifier *modifier = *i;
            wxPanel *panel = modifier->getControlPanel(this->list_book);
            list_book->AddPage(panel, wxString::FromUTF8(modifier->getName()));
            panel->Show();
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
            list_book->RemovePage(0);
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
        list_book->AddPage(modifier->getControlPanel(list_book), modifier->getName());
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
        list_book->RemovePage(index);
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
