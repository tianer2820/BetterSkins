#if !defined(LAYER_VIEWER_H)
#define LAYER_VIEWER_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../dataStructure/skin.hpp"


wxDECLARE_EVENT(EVT_LAYER_CHANGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_LAYER_CHANGE, wxCommandEvent);
/**
 * To view the whole skin layer stack.
 * call loadSkin to load a skin document
 * refreshNames to update the layer names
 * clear to unload
 * 
 * this class sends two events:
 * EVT_LAYER_UPDATE: when layer need to be redreawed
 * EVT_LAYER_CHANGE: new layer is selected
 */
class LayerViewer : public wxPanel
{
public:
    LayerViewer(wxWindow *parent, wxWindowID id = wxID_ANY) : wxPanel(parent, id)
    {
        wxBoxSizer *main_box = new wxBoxSizer(wxVERTICAL);
        list_box = new wxListBox(this, wxID_ANY);
        Bind(wxEVT_LISTBOX, &LayerViewer::onLayerChange, this, list_box->GetId());
        main_box->Add(list_box, 1, wxBOTTOM | wxEXPAND, 5);

        wxBoxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
        wxButton *but_add = new wxButton(this, wxID_ANY, wxString::FromUTF8("+"));
        wxButton *but_delete = new wxButton(this, wxID_ANY, wxString::FromUTF8("-"));
        wxButton *but_up = new wxButton(this, wxID_ANY, wxString::FromUTF8("UP"));
        wxButton *but_down = new wxButton(this, wxID_ANY, wxString::FromUTF8("DOWN"));
        wxButton *but_duplicate = new wxButton(this, wxID_ANY, wxString::FromUTF8("DUP"));
        but_add->SetMinSize(wxSize(20, 20));
        but_delete->SetMinSize(wxSize(20, 20));
        but_up->SetMinSize(wxSize(20, 20));
        but_down->SetMinSize(wxSize(20, 20));
        but_duplicate->SetMinSize(wxSize(20, 20));
        Bind(wxEVT_BUTTON, &LayerViewer::onAdd, this, but_add->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onDelete, this, but_delete->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onMoveUp, this, but_up->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onMoveDown, this, but_down->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onDuplicate, this, but_duplicate->GetId());
        button_box->Add(but_add, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_delete, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_up, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_down, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_duplicate, 1, wxALL | wxEXPAND, 2);
        main_box->Add(button_box, 0, wxEXPAND);

        SetSizer(main_box);
    }
    void loadSkin(Skin *skin)
    {
        current_document = skin;
        // delete all existing items
        list_box->Clear();
        // add new items
        int len = skin->getLayerNum();
        for (int i = 0; i < len; i++)
        {
            Layer *layer = skin->getLayer(i);
            list_box->Insert(layer->getName(), 0);
        }
        setActiveLayer();
    }
    void refreshNames()
    {
        int len = list_box->GetCount();
        for (int i = 0; i < len; i++)
        {
            int index = len - 1 - i;
            string new_name = current_document->getLayer(index)->getName();
            if (list_box->GetString(i).ToStdString() != new_name)
            {
                list_box->SetString(i, wxString::FromUTF8(new_name));
            }
        }
    }
    void clear()
    {
        current_document = NULL;
        list_box->Clear();
    }

    void setActiveLayer(int index = 0)
    {
        if (list_box->GetCount() != 0 && index < list_box->GetCount())
        {
            list_box->SetSelection(index);
            sendLayerChangeEvent();
        }
    }

protected:
    Skin *current_document = NULL;
    wxListBox *list_box = NULL;
    void sendLayerChangeEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_LAYER_CHANGE, GetId());
        event->SetEventObject(this);
        int index = list_box->GetCount() - 1 - list_box->GetSelection();
        if (list_box->GetSelection() == -1)
        {
            index = -1;
        }
        event->SetInt(index);
        wxQueueEvent(GetEventHandler(), event);
    }
    void onLayerChange(wxCommandEvent &event)
    {
        sendLayerChangeEvent();
    }
    void onAdd(wxCommandEvent &event)
    {
        if (current_document == NULL)
        {
            return;
        }
        wxSize size = current_document->getLayerSize();
        Layer *layer = new Layer("new layer", size);
        current_document->addLayer(layer);
        list_box->Insert(layer->getName(), 0);
        list_box->SetSelection(0);
        sendLayerChangeEvent();
        sendUpdateEvent();
    }
    void onDelete(wxCommandEvent &event)
    {
        int index = list_box->GetSelection();
        if (current_document == NULL || index == -1)
        {
            return;
        }
        int index2 = list_box->GetCount() - 1 - index;
        list_box->Delete(index);
        if (index - 1 >= 0)
        {
            list_box->SetSelection(index - 1);
        }
        else if (list_box->GetCount() != 0)
        {
            list_box->SetSelection(0);
        }

        current_document->deleteLayer(index2);
        sendLayerChangeEvent();
        sendUpdateEvent();
    }
    void onMoveUp(wxCommandEvent &event)
    {
        int index = list_box->GetSelection();
        if (current_document == NULL || index == -1 || index == 0)
        {
            return;
        }
        // change list box
        wxString name = list_box->GetString(index);
        list_box->Delete(index);
        list_box->Insert(name, index - 1);
        list_box->SetSelection(index - 1);
        // change document
        index = list_box->GetCount() - index - 1;
        current_document->moveLayer(index, index + 1);
        sendUpdateEvent();
    }
    void onMoveDown(wxCommandEvent &event)
    {
        int index = list_box->GetSelection();
        if (current_document == NULL || index == -1 || index == list_box->GetCount() - 1)
        {
            return;
        }
        // change list box
        wxString name = list_box->GetString(index);
        list_box->Delete(index);
        list_box->Insert(name, index + 1);
        list_box->SetSelection(index + 1);
        // change document
        index = list_box->GetCount() - index - 1;
        current_document->moveLayer(index, index - 1);
        sendUpdateEvent();
    }
    void onDuplicate(wxCommandEvent &event)
    {
        int index = list_box->GetSelection();
        int count = list_box->GetCount();
        if (current_document == NULL || index == -1)
        {
            return;
        }
        // change list box
        wxString name = list_box->GetString(index);
        list_box->Insert(name + "(copy)", index);
        list_box->SetSelection(index);
        sendLayerChangeEvent();
        // change document
        index = count - index - 1;
        Layer *layer = current_document->getLayer(index);
        Layer *new_layer = new Layer(*layer);
        new_layer->setName(new_layer->getName() + "(copy)");
        current_document->addLayer(new_layer, index + 1);
        sendUpdateEvent();
    }
    void sendUpdateEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_LAYER_UPDATE, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
};


#endif // LAYER_VIEWER_H
