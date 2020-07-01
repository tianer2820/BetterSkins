#if !defined(LAYER_VIEWER_H)
#define LAYER_VIEWER_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dataview.h>

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
        list_box = new wxDataViewListCtrl(this, wxID_ANY);
        list_box->AppendTextColumn(_T("Layer Name"), wxDATAVIEW_CELL_EDITABLE);
        list_box->AppendToggleColumn(_T("Visable"));

        Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &LayerViewer::onLayerChange, this, list_box->GetId());
        Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, &LayerViewer::onRename, this, list_box->GetId());
        Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &LayerViewer::onDoubleClick, this, list_box->GetId());
        Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, &LayerViewer::onToggleVisable, this, list_box->GetId());
        main_box->Add(list_box, 1, wxBOTTOM | wxEXPAND, 5);
        
        // buttons
        wxImage icon;
        wxString icon_path = _T("./resources/icons/");
        wxBoxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
        //add
        icon.LoadFile(icon_path + _T("add.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_add = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_add->SetToolTip(_T("Add a new layer"));
        //delete
        icon.LoadFile(icon_path + _T("delete.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_delete = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_delete->SetToolTip(_T("Delete the selected layer"));
        //up
        icon.LoadFile(icon_path + _T("up.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_up = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_up->SetToolTip(_T("Move this layer up"));
        //down
        icon.LoadFile(icon_path + _T("down.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_down = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_down->SetToolTip(_T("Move this layer down"));
        //duplicate
        icon.LoadFile(icon_path + _T("dup.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_duplicate = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_duplicate->SetToolTip(_T("Duplicate the selected layer"));
        //merge
        icon.LoadFile(icon_path + _T("merge.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        wxBitmapButton *but_merge = new wxBitmapButton(this, wxID_ANY, wxBitmap(icon));
        but_merge->SetToolTip(_T("Merge this layer to the lower"));

        but_add->SetMinSize(wxSize(20, 20));
        but_delete->SetMinSize(wxSize(20, 20));
        but_up->SetMinSize(wxSize(20, 20));
        but_down->SetMinSize(wxSize(20, 20));
        but_duplicate->SetMinSize(wxSize(20, 20));
        but_merge->SetMinSize(wxSize(20, 20));
        Bind(wxEVT_BUTTON, &LayerViewer::onAdd, this, but_add->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onDelete, this, but_delete->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onMoveUp, this, but_up->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onMoveDown, this, but_down->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onDuplicate, this, but_duplicate->GetId());
        Bind(wxEVT_BUTTON, &LayerViewer::onMerge, this, but_merge->GetId());
        button_box->Add(but_add, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_delete, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_up, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_down, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_duplicate, 1, wxALL | wxEXPAND, 2);
        button_box->Add(but_merge, 1, wxALL | wxEXPAND, 2);
        main_box->Add(button_box, 0, wxEXPAND);

        SetSizer(main_box);
    }
    void loadSkin(Skin *skin)
    {
        current_document = skin;
        // delete all existing items
        list_box->DeleteAllItems();
        // add new items
        int len = skin->getLayerNum();
        for (int i = 0; i < len; i++)
        {
            Layer *layer = skin->getLayer(i);
            wxVector<wxVariant> item;
            item.push_back(wxVariant(layer->getName()));
            item.push_back(wxVariant(layer->getVisible()));
            list_box->PrependItem(item);
        }
        setActiveLayer();
    }
    void refreshNames()
    {
        int len = list_box->GetItemCount();
        for (int i = 0; i < len; i++)
        {
            int index = len - 1 - i;
            string new_name = current_document->getLayer(index)->getName();
            wxVariant value;
            list_box->GetValue(value, i, 0);
            if (value.GetString().ToStdString() != new_name)
            {
                value = new_name;
                list_box->SetValue(value, i, 0);
            }
        }
    }
    void clear()
    {
        current_document = NULL;
        list_box->DeleteAllItems();
    }

    void setActiveLayer(int index = 0)
    {
        if (list_box->GetItemCount() != 0 && index < list_box->GetItemCount())
        {
            list_box->SelectRow(index);
            sendLayerChangeEvent();
        }
    }
    int getActiveLayer(){
        return list_box->GetSelectedRow();
    }
protected:
    Skin *current_document = NULL;
    wxDataViewListCtrl *list_box = NULL;
    void sendLayerChangeEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_LAYER_CHANGE, GetId());
        event->SetEventObject(this);
        int selected_row = list_box->GetSelectedRow();
        int index;
        if (selected_row == wxNOT_FOUND)
        {
            index = -1;
        }
        else
        {
            index = list_box->GetItemCount() - 1 - selected_row;
        }

        event->SetInt(index);
        wxQueueEvent(GetEventHandler(), event);
    }
    void sendLayerUpdateEvent(){
        wxCommandEvent* event = new wxCommandEvent(EVT_LAYER_UPDATE, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
    void onLayerChange(wxCommandEvent &event)
    {
        if(list_box->GetSelectedRow() == wxNOT_FOUND && list_box->GetItemCount() > 0){
            // deselected, force select first layer. You must select something...
            list_box->SelectRow(0);
        }
        sendLayerChangeEvent();
    }
    void onDoubleClick(wxDataViewEvent &event){
        if(event.GetColumn() == 0){
            list_box->StartEditor(event.GetItem(), 0);
        } else if(event.GetColumn() == 1){
            int row = list_box->GetSelectedRow();
            bool value = list_box->GetToggleValue(row, 1);
            list_box->SetToggleValue(!value, row, 1);
        }
    }
    void onRename(wxDataViewEvent &event){
        int col = event.GetColumn();
        if(col != 0){
            // some other things are changed..
            event.Skip();
            return;
        }
        if(current_document != nullptr){
            Layer* active_layer = current_document->getLayer(list_box->GetItemCount() - list_box->GetSelectedRow() - 1);
            active_layer->setName(event.GetValue().GetString().ToStdString());
            sendLayerChangeEvent();
        }
    }
    void onToggleVisable(wxDataViewEvent &event){
        if (event.GetColumn() != 1)
        {
            // some thing else is changed, ignore
            event.Skip();
            return;
        }
        if(current_document != nullptr){
            Layer* active_layer = current_document->getLayer(list_box->GetItemCount() - list_box->GetSelectedRow() - 1);
            active_layer->setVisible(list_box->GetToggleValue(list_box->GetSelectedRow(), 1));
            sendLayerChangeEvent();
            sendLayerUpdateEvent();
        }
        
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
        wxVector<wxVariant> item;
        item.push_back(wxVariant(layer->getName()));
        item.push_back(wxVariant(layer->getVisible()));
        list_box->PrependItem(item);
        list_box->SelectRow(0);
        sendLayerChangeEvent();
        sendUpdateEvent();
    }
    void onDelete(wxCommandEvent &event)
    {
        int index = list_box->GetSelectedRow();
        if (current_document == NULL || index == wxNOT_FOUND)
        {
            return;
        }
        int index2 = list_box->GetItemCount() - 1 - index;
        list_box->DeleteItem(index);
        if (index - 1 >= 0)
        {
            list_box->SelectRow(index - 1);
        }
        else if (list_box->GetItemCount() != 0)
        {
            list_box->SelectRow(0);
        }

        current_document->deleteLayer(index2);
        sendLayerChangeEvent();
        sendUpdateEvent();
    }
    void onMoveUp(wxCommandEvent &event)
    {
        int index = list_box->GetSelectedRow();
        if (current_document == NULL || index == wxNOT_FOUND || index == 0)
        {
            return;
        }
        // change list box
        wxVector<wxVariant> item;
        wxVariant value;
        for (int i = 0; i < list_box->GetColumnCount(); i++)
        {
            list_box->GetValue(value, index, i);
            item.push_back(value);
        }
        list_box->DeleteItem(index);
        list_box->InsertItem(index - 1, item);
        list_box->SelectRow(index - 1);
        // change document
        index = list_box->GetItemCount() - index - 1;
        current_document->moveLayer(index, index + 1);
        sendUpdateEvent();
    }
    void onMoveDown(wxCommandEvent &event)
    {
        int index = list_box->GetSelectedRow();
        if (current_document == NULL || index == wxNOT_FOUND || index == list_box->GetItemCount() - 1)
        {
            return;
        }
        // change list box
        wxVector<wxVariant> item;
        wxVariant value;
        for (int i = 0; i < list_box->GetColumnCount(); i++)
        {
            list_box->GetValue(value, index, i);
            item.push_back(value);
        }
        list_box->DeleteItem(index);
        list_box->InsertItem(index + 1, item);
        list_box->SelectRow(index + 1);
        // change document
        index = list_box->GetItemCount() - index - 1;
        current_document->moveLayer(index, index - 1);
        sendUpdateEvent();
    }
    void onDuplicate(wxCommandEvent &event)
    {
        int index = list_box->GetSelectedRow();
        int count = list_box->GetItemCount();
        if (current_document == NULL || index == wxNOT_FOUND)
        {
            return;
        }
        // change list box
        wxVector<wxVariant> item;
        wxVariant value;
        for (int i = 0; i < list_box->GetColumnCount(); i++)
        {
            list_box->GetValue(value, index, i);
            item.push_back(value);
        }
        item.at(0) = item.at(0).GetString() + "(copy)";
        list_box->InsertItem(index, item);
        list_box->SelectRow(index);
        sendLayerChangeEvent();
        // change document
        index = count - index - 1;
        Layer *layer = current_document->getLayer(index);
        Layer *new_layer = new Layer(*layer);
        new_layer->setName(new_layer->getName() + "(copy)");
        current_document->addLayer(new_layer, index + 1);
        sendUpdateEvent();
    }
    void onMerge(wxCommandEvent &event){
        int index = list_box->GetSelectedRow();
        if (current_document == NULL || index == wxNOT_FOUND)
        {
            return;
        }
        int index_layer = list_box->GetItemCount() - 1 - index;
        if(index_layer == 0){
            return; // can't merge down if this is the bottom layer
        }
        list_box->DeleteItem(index);
        list_box->SelectRow(index);
        
        Layer* layer = current_document->getLayer(index_layer);
        Layer* lower = current_document->getLayer(index_layer - 1);
        wxImage upper_img = layer->render();
        wxImage* lower_img = lower->getImage();
        alphaOver(*lower_img, upper_img);

        current_document->deleteLayer(index_layer);
        sendLayerChangeEvent();
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
