#if !defined(TOOL_BOX_H)
#define TOOL_BOX_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../dataStructure/tools/Tool.hpp"
#include "../dataStructure/tools/solidPen.hpp"
#include "../dataStructure/tools/noisePen.hpp"
#include "../dataStructure/tools/eraser.hpp"
#include "../dataStructure/tools/lightenDarken.hpp"
#include "../dataStructure/tools/dropper.hpp"
#include "../dataStructure/tools/moveTool.hpp"
#include "../dataStructure/tools/selectionTool.hpp"

wxDECLARE_EVENT(EVT_BUTTON_LIST_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_BUTTON_LIST_CHANGED, wxCommandEvent);
/**
 * A box with single choice buttons
 * this class send this event:
 * EVT_BUTTON_LIST_CHANGED
 */
class ToolButtonList : public wxScrolledWindow
{
public:
    ToolButtonList(wxWindow *parent, wxWindowID id = wxID_ANY) : wxScrolledWindow(parent, id)
    {
        this->SetWindowStyle(wxVSCROLL);
        this->SetScrollRate(0, 5);

        wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
        grid = new wxGridSizer(3);

        box->Add(grid, 0);
        this->SetSizer(box);

        Bind(wxEVT_SIZE, &ToolButtonList::onSize, this);
        Bind(wxEVT_TOGGLEBUTTON, &ToolButtonList::onButton, this);
    }
    /**
     * button must have this object as its parent!
     */
    void addButton(wxBitmapToggleButton *button)
    {
        if (button == NULL)
        {
            return;
        }
        button->SetMaxSize(wxSize(20, 20));
        button->SetMinSize(wxSize(20, 20));
        button_list.push_back(button);
        grid->Add(button, 0, wxALL, 1);

        // set default selection
        if(button_list.size() == 1){
            button_list.at(0)->SetValue(true);
            selection = 0;
            sendButtonListChangedEvent();
        }
    }
    int getSelection()
    {
        return selection;
    }
    void setSelection(int i){
        if(i < 0 || i >= button_list.size()){
            if(button_list.size() > 0){
                selection = 0;
            } else{
                selection = -1;
            }
        } else{
            selection = i;
        }
        int size = button_list.size();
        for (int index = 0; index < size; index++)
        {
            if(index == selection){
                button_list.at(index)->SetValue(true);
            } else{
                button_list.at(index)->SetValue(false);
            }
        }
    }

protected:
    wxGridSizer *grid;
    vector<wxBitmapToggleButton *> button_list;
    int selection = -1;
    void onButton(wxCommandEvent &event)
    {
        wxBitmapToggleButton *obj = dynamic_cast<wxBitmapToggleButton *>(event.GetEventObject());
        if (obj == NULL)
        {
            return;
        }
        int index = 0;
        for (auto i = button_list.begin(); i != button_list.end(); i++)
        {
            wxBitmapToggleButton *button = *i;
            if (button != obj)
            {
                button->SetValue(false);
            }
            else
            {
                button->SetValue(true);
                selection = index;
            }
            index += 1;
        }
        sendButtonListChangedEvent();
    }
    void onSize(wxSizeEvent &event)
    {
        int size;
        if (button_list.size() > 0)
        {
            size = button_list.at(0)->GetBestWidth(10) + 2;
        }
        else
        {
            size = 10;
        }
        int width = GetClientSize().GetWidth();
        int cols = max(1, width / size);
        grid->SetCols(cols);
        this->FitInside();
        event.Skip();
    }
    void sendButtonListChangedEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_BUTTON_LIST_CHANGED, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
    wxSize DoGetBestSize() const override
    {
        return GetSize();
    }
};

/**
 * this event is sent when a tool option widget is altered by user.
 * the string data is used as the option name;
 * the int data is used as the option value;
 */
wxDECLARE_EVENT(EVT_TOOL_CHANGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_TOOL_CHANGE, wxCommandEvent);
/**
 * the tool/brush box.
 * this class will send this event:
 * EVT_TOOL_CHANGE: when a new tool is selected
 */
class ToolBox : public wxSplitterWindow
{
public:
    ToolBox(wxWindow *parent, wxWindowID id = wxID_ANY) : wxSplitterWindow(parent, id)
    {
        this->SetWindowStyle(wxSP_3DSASH | wxSP_LIVE_UPDATE | wxSP_NO_XP_THEME);

        // create layout widgets
        button_list = new ToolButtonList(this);
        wxSplitterWindow *splitter2 = new wxSplitterWindow(this);
        splitter2->SetWindowStyle(wxSP_3DSASH | wxSP_LIVE_UPDATE | wxSP_NO_XP_THEME);
        wxScrolledWindow *common_options = new wxScrolledWindow(splitter2);
        common_options->SetScrollRate(0, 5);
        option_book = new wxSimplebook(splitter2, wxID_ANY);

        // prepare common ctrl panel
        wxGridSizer *grid = new wxGridSizer(2);
        wxStaticText *label = new wxStaticText(common_options, wxID_ANY, _T("Size:"));
        size_ctrl = new wxSpinCtrl(common_options);
        size_ctrl->SetMin(1);
        size_ctrl->SetMax(16);
        Bind(wxEVT_SPINCTRL, &ToolBox::onSizeChange, this, size_ctrl->GetId());
        grid->Add(label, 1, wxALL, 2);
        grid->Add(size_ctrl, 0, wxALL, 2);

        common_options->SetSizer(grid);

        // prepare special ctrl book
        wxImage icon;
        wxString icon_path = _T("./resources/icons/");
        wxScrolledWindow *panel;
        wxBitmapToggleButton *button;

        //------------------------------------------------

        // solid pen
        tool_list.push_back(new SolidPen());
        icon.LoadFile(icon_path + _T("pen.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        panel = new wxScrolledWindow(option_book);
        panel->SetScrollRate(0, 5);

        option_book->AddPage(panel, _T("Pen"));
        button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(icon));
        button->SetToolTip(_T("Simple pixel pen"));
        button_list->addButton(button);

        // noise pen
        tool_list.push_back(new NoisePen());
        icon.LoadFile(icon_path + _T("noise.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        panel = new wxScrolledWindow(option_book);
        panel->SetScrollRate(0, 5);

        option_book->AddPage(panel, _T("Noise"));
        button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(icon));
        button->SetToolTip(_T("Noise pen"));
        button_list->addButton(button);

        // eraser
        tool_list.push_back(new Eraser());
        icon.LoadFile(icon_path + _T("eraser.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        panel = new wxScrolledWindow(option_book);
        panel->SetScrollRate(0, 5);

        option_book->AddPage(panel, _T("Eraser"));
        button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(icon));
        button->SetToolTip(_T("Eraser"));
        button_list->addButton(button);

        // Lightness brush
        lighten_darken_tool = new LightenDarkenTool();
        tool_list.push_back(lighten_darken_tool);
        icon.LoadFile(icon_path + _T("lighten_darken.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        panel = new wxScrolledWindow(option_book);
        panel->SetScrollRate(0, 5);
        grid = new wxGridSizer(2);

        wxSpinCtrl *lightness_spin = new wxSpinCtrl(panel);
        lightness_spin->SetMin(-255);
        lightness_spin->SetMax(255);
        lightness_spin->SetValue(10);
        label = new wxStaticText(panel, wxID_ANY, _T("Lightness:"));
        grid->Add(label, 0, 0, 2);
        grid->Add(lightness_spin, 0, 0, 2);
        Bind(wxEVT_SPINCTRL, &ToolBox::onLightnessChange, this, lightness_spin->GetId());

        label = new wxStaticText(panel, wxID_ANY, _T("Incremental:"));
        incremental_box = new wxCheckBox(panel, wxID_ANY, _T("on"));
        grid->Add(label, 0, 0, 2);
        grid->Add(incremental_box, 0, 0, 2);
        Bind(wxEVT_CHECKBOX, &ToolBox::onIncremental, this, incremental_box->GetId());

        panel->SetSizer(grid);
        option_book->AddPage(panel, _T("Lightness"));
        button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(icon));
        button->SetToolTip(_T("Lightness pen"));
        button_list->addButton(button);

        // dropper
        tool_list.push_back(new Dropper());
        icon.LoadFile(icon_path + _T("dropper.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        panel = new wxScrolledWindow(option_book);
        panel->SetScrollRate(0, 5);

        option_book->AddPage(panel, _T("Dropper"));
        button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(icon));
        button->SetToolTip(_T("Color picker"));
        button_list->addButton(button);

        // move tool
        tool_list.push_back(new MoveTool());
        icon.LoadFile(icon_path + _T("move.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        panel = new wxScrolledWindow(option_book);
        panel->SetScrollRate(0, 5);

        option_book->AddPage(panel, _T("MoveTool"));
        button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(icon));
        button->SetToolTip(_T("Move Tool"));
        button_list->addButton(button);

        // selection tool
        tool_list.push_back(new SelectionTool());
        icon.LoadFile(icon_path + _T("selection.png"));
        icon.Rescale(16, 16, wxIMAGE_QUALITY_BILINEAR);
        panel = new wxScrolledWindow(option_book);
        panel->SetScrollRate(0, 5);

        option_book->AddPage(panel, _T("SelectionTool"));
        button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(icon));
        button->SetToolTip(_T("Box selection"));
        button_list->addButton(button);

        // do the layout
        splitter2->SplitVertically(common_options, option_book, 100);
        this->SplitVertically(button_list, splitter2, 120);

        Bind(EVT_BUTTON_LIST_CHANGED, &ToolBox::onButtonChange, this);
    }
    ~ToolBox()
    {
        for (auto i = tool_list.begin(); i != tool_list.end(); i++)
        {
            delete *i;
        }
    }
    /**
     * Get the active tool. Don't delete the returned object.
     */
    Tool *getTool()
    {
        int i = button_list->getSelection();
        if (i == -1)
        {
            return NULL;
        }
        return tool_list.at(i);
    }
    void setColor(Color *color)
    {
        int rgb[3];
        color->getRGB(rgb);
        for (auto i = tool_list.begin(); i != tool_list.end(); i++)
        {

            (*i)->setProperty("R", rgb[0]);
            (*i)->setProperty("G", rgb[1]);
            (*i)->setProperty("B", rgb[2]);
            (*i)->setProperty("A", color->getAlpha());
        }
    }
    void setSelection(int index){
        button_list->setSelection(index);
        option_book->ChangeSelection(button_list->getSelection());
        //update common options
        size_ctrl->SetValue(getTool()->getProperty("SIZE"));
    }

protected:
    ToolButtonList *button_list;
    vector<Tool *> tool_list;
    wxSimplebook *option_book;
    bool independent_size = false;
    // common option ctrls
    wxSpinCtrl *size_ctrl;

    // special ctrls and brushes
    LightenDarkenTool *lighten_darken_tool;
    wxCheckBox *incremental_box;
    void onLightnessChange(wxSpinEvent &event)
    {
        lighten_darken_tool->setProperty("LIGHTNESS", event.GetPosition());
    }
    void onIncremental(wxCommandEvent &event)
    {
        int is_incremental = 0;
        if (incremental_box->GetValue())
        {
            is_incremental = 1;
        }
        lighten_darken_tool->setProperty("INCREMENTAL", is_incremental);
    }

    void onSizeChange(wxSpinEvent &event)
    {
        // process the options;
        if (!independent_size)
        {
            for (auto i = tool_list.begin(); i != tool_list.end(); i++)
            {
                (*i)->setProperty("SIZE", event.GetPosition());
            }
        }
        else
        {
            getTool()->setProperty("SIZE", event.GetPosition());
        }
    }
    void onButtonChange(wxCommandEvent &event)
    {
        option_book->ChangeSelection(button_list->getSelection());
        sendToolChangeEvent();

        //update common options
        size_ctrl->SetValue(getTool()->getProperty("SIZE"));
    }
    void sendToolChangeEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_TOOL_CHANGE, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
    wxSize DoGetBestSize() const override
    {
        return GetSize();
    }
};


#endif // TOOL_BOX_H
