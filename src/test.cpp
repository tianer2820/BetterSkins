#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listbook.h>
#include <wx/splitter.h>
#include <wx/tglbtn.h>
#include <wx/simplebook.h>
#include <wx/file.h>

#include "json.hpp"
using json = nlohmann::json;

#include "customUI/colorPickers/advColorPicker.hpp"
#include "customUI/skinBrowser.h"
#include "customUI/layerModifierUIs/colorRampCtrl.hpp"
#include "customUI/LayerControlPanel.hpp"
#include "customUI/LayerViewer.hpp"
#include "customUI/referenceWindow.hpp"

#include "dataStructure/skin.hpp"
#include "dataStructure/command.hpp"
#include "dataStructure/layerModifier.hpp"
#include "dataStructure/layer.hpp"
#include "dataStructure/skin.hpp"
#include "dataStructure/SkinFormat.hpp"
#include "dataStructure/tools/Tool.hpp"
#include "dataStructure/commandManager.hpp"

#include "dataStructure/tools/solidPen.hpp"
#include "dataStructure/tools/noisePen.hpp"
#include "dataStructure/tools/eraser.hpp"

#include <vector>
#include <map>
#include <random>
using namespace std;

wxDECLARE_EVENT(EVT_CANVAS_PAINT, wxCommandEvent);
wxDEFINE_EVENT(EVT_CANVAS_PAINT, wxCommandEvent);
/**
 * To Display the skin, and draw on it.
 * 
 * Call loadSkin to load and display a skin, call loadLayer to set the active layer.
 * Call setPen to set the active pen.
 * 
 * This class emmits this event:
 * EVT_CANVAS_PAINT: when the canvas starts to paint
 * 
 * todo:
 * different tool type: pen, color picker, selection, move
 * emmit new event: color picked
 */
class Canvas : public wxWindow
{
public:
    Canvas(wxWindow *parent, wxWindowID id = wxID_ANY) : wxWindow(parent, id)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &Canvas::onPaint, this);
        Bind(wxEVT_LEFT_DOWN, &Canvas::onMouse, this);
        Bind(wxEVT_MOTION, &Canvas::onMouse, this);
        Bind(wxEVT_LEFT_UP, &Canvas::onMouse, this);
        Bind(wxEVT_MIDDLE_DOWN, &Canvas::onMouse, this);
        Bind(wxEVT_MIDDLE_UP, &Canvas::onMouse, this);
        Bind(wxEVT_MOUSEWHEEL, &Canvas::onMouse, this);
        // Bind(wxEVT_COMMAND_RIGHT_CLICK, &Canvas::onMouse, this);

        initSkinFormat();
    }
    ~Canvas()
    {
    }

    void initSkinFormat()
    {
        // read file
        wxFile file;
        bool file_open = file.Open(_T("resources/formats.json"));
        if (!file_open)
        {
            return;
        }
        wxString str;
        bool read = file.ReadAll(&str);

        json j = json::parse(str.ToStdString());
        if (j.is_object())
        {
            // get the skins dict
            json skins = j["skins"];
            if (skins.is_array())
            {
                int size = skins.size();
                for (int i = 0; i < size; i++)
                {
                    //each skin type
                    json format_block = skins[i];
                    if (format_block.is_array())
                    {
                        // each skin type
                        int block_length = format_block.size();
                        SkinFormat skin;
                        for (int j = 0; j < block_length; j++)
                        {
                            // each text block
                            json block = format_block[j];
                            string block_label = block["text"].get<string>();
                            vector<int> block_size = block["size"].get<vector<int>>();

                            TextBlock t_block;
                            t_block.name = block_label;
                            t_block.top = block_size.at(0);
                            t_block.left = block_size.at(1);
                            t_block.width = block_size.at(2);
                            t_block.height = block_size.at(3);
                            skin.push_back(t_block);
                        }
                        skin_formats.push_back(skin);
                    }
                }
            }
        }
    }

    /**
     * call with nullptr to unload
     */
    void loadSkin(Skin *skin)
    {
        // this class won't delete the loaded skin. You should save and delete it.
        current_skin = skin;
        Refresh();
        Update();
    }
    /**
     * pass an index of -1 to unload the layer
     */
    void loadLayer(int index)
    {
        if (index == -1)
        {
            current_layer = NULL;
            return;
        }
        if (current_skin != NULL)
        {
            current_layer = current_skin->getLayer(index);
        }
    }

    /**
     * Set the pen object. This will not be deleted after use!
     */
    void setPen(Tool *pen)
    {
        current_pen = pen;
    }

protected:
    Skin *current_skin = NULL;
    Layer *current_layer = NULL;
    bool is_drawing = false;
    Tool *current_pen = NULL;

    vector<SkinFormat> skin_formats;

    bool is_dragging = false;
    int offset_x = 0;
    int offset_y = 0;
    double drag_point_x = 0;
    double drag_point_y = 0;
    double scale = 10;

    void screenToImage(int x, int y, double &out_x, double &out_y)
    {
        out_x = (x - offset_x) / scale;
        out_y = (y - offset_y) / scale;
    }
    void screenToImage(int x, int y, int &out_x, int &out_y)
    {
        double x1, y1;
        screenToImage(x, y, x1, y1);
        out_x = static_cast<int>(x1);
        if (x1 < 0)
        {
            out_x -= 1;
        }
        out_y = static_cast<int>(y1);
        if (y1 < 0)
        {
            out_y -= 1;
        }
    }
    void imageToScreen(double x, double y, int &out_x, int &out_y)
    {
        out_x = x * scale + offset_x;
        out_y = y * scale + offset_y;
    }

    void onMouse(wxMouseEvent &event)
    {
        if (current_skin == NULL || current_layer == NULL)
        {
            return;
        }
        int x = event.GetX();
        int y = event.GetY();
        if (!is_dragging)
        {
            if (event.LeftDown())
            {
                // start drawing a stroke
                if (current_pen != NULL)
                {
                    this->CaptureMouse();
                    is_drawing = true;
                    current_pen->setLayer(current_layer);
                    int x1, y1;
                    screenToImage(x, y, x1, y1);
                    current_pen->moveTo(x1, y1);
                    current_pen->penDown();
                    sendStartPaintEvent();
                }
            }
            if (is_drawing)
            {
                if (x < 0 || x >= 640 || y < 0 || y >= 640)
                {
                    // don't paint
                }
                else
                {
                    if (current_pen != NULL)
                    {
                        int x1, y1;
                        screenToImage(x, y, x1, y1);
                        current_pen->moveTo(x1, y1);
                    }
                }
                if (event.LeftUp())
                {
                    // finish drawing
                    this->ReleaseMouse();
                    is_drawing = false;
                    current_pen->penUp();
                }
            }
        }
        if (!is_drawing)
        {
            if (event.MiddleDown())
            {
                // start drag
                screenToImage(event.GetX(), event.GetY(), drag_point_x, drag_point_y);
                is_dragging = true;
                SetCursor(wxCursor(wxCURSOR_SIZING));
                if (!is_drawing)
                {
                    CaptureMouse();
                }
            }
            if (is_dragging)
            {
                offset_x = event.GetX() - drag_point_x * scale;
                offset_y = event.GetY() - drag_point_y * scale;
                if (event.MiddleUp())
                {
                    // end drag
                    SetCursor(wxCursor(wxCURSOR_DEFAULT));
                    if (!is_drawing)
                    {
                        ReleaseMouse();
                    }
                    is_dragging = false;
                }
            }
        }
        int wheel_delta = event.GetWheelDelta();
        if (event.GetWheelRotation() != 0)
        {
            // zoom
            int rotation = event.GetWheelRotation() / wheel_delta;
            double factor = pow(1.2, rotation);
            int s_offset_x = offset_x - x;
            int s_offset_y = offset_y - y;
            offset_x += s_offset_x * (factor - 1);
            offset_y += s_offset_y * (factor - 1);
            scale *= factor;
        }

        Refresh();
        event.Skip();
    }
    void onPaint(wxPaintEvent &event)
    {
        // fill alpha background
        wxImage img(GetSize());
        img.InitAlpha();
        Color::drawAlpha(img);

        wxAutoBufferedPaintDC dc(this);
        dc.DrawBitmap(wxBitmap(img), 0, 0);
        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

        // render skin
        if (current_skin != NULL)
        {
            wxImage img2 = current_skin->render();
            u_char *source_data = img2.GetData();
            u_char *source_alpha = img2.GetAlpha();
            int source_w = img2.GetWidth();
            int source_h = img2.GetHeight();

            wxImage img_screen = wxImage(this->GetSize());
            img_screen.InitAlpha();
            u_char *data = img_screen.GetData();
            u_char *alpha = img_screen.GetAlpha();
            int w = img_screen.GetWidth();
            int h = img_screen.GetHeight();

            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    int pixel_x, pixel_y;
                    screenToImage(x, y, pixel_x, pixel_y);
                    int index = y * w + x;
                    int source_index = pixel_y * source_w + pixel_x;
                    if (pixel_x < 0 || pixel_y < 0 ||
                        pixel_x >= source_w || pixel_y >= source_h)
                    {
                        // out of range, set alpha
                        alpha[index] = 72;
                        continue;
                    }
                    // else copy data and alpha
                    for (int i = 0; i < 3; i++)
                    {
                        data[index * 3 + i] = source_data[source_index * 3 + i];
                    }
                    alpha[index] = source_alpha[source_index];
                }
            }

            gc->DrawBitmap(wxBitmap(img_screen, 32), 0, 0, w, h);

            //draw outline
            SkinFormat format;
            switch (this->current_skin->getSkinType())
            {
            case SkinType::STEVE:
                format = this->skin_formats.at(0);
                break;
            case SkinType::ALEX:
                format = this->skin_formats.at(1);
                break;
            case SkinType::STEVE_MIN:
                format = this->skin_formats.at(2);
                break;
            case SkinType::ALEX_MIN:
                format = this->skin_formats.at(3);
            default:
                break;
            }
            for (auto i = format.begin(); i != format.end(); i++)
            {
                TextBlock block = *i;
                int block_screen_x, block_screen_y;
                imageToScreen(block.left, block.top, block_screen_x, block_screen_y);

                // draw the shadow first
                gc->SetPen(wxPen(wxColour(0, 0, 0), 1));
                gc->SetFont(*wxNORMAL_FONT, wxColour(0, 0, 0));
                gc->DrawRectangle(block_screen_x + 1,
                                  block_screen_y + 1,
                                  block.width * scale,
                                  block.height * scale);
                gc->DrawText(wxString(block.name), block_screen_x + 1,
                             block_screen_y + 1);

                // now draw the white line
                gc->SetPen(wxPen(wxColour(255, 255, 255), 1));
                gc->SetFont(*wxNORMAL_FONT, wxColour(255, 255, 255));
                gc->DrawRectangle(block_screen_x,
                                  block_screen_y,
                                  block.width * scale,
                                  block.height * scale);
                gc->DrawText(wxString(block.name), block_screen_x,
                             block_screen_y);
            }
        }
        // render brush square
        if (current_pen != nullptr)
        {
            int size = current_pen->getProperty("SIZE");
            if (size > 0)
            {
                int mouse_img_x, mouse_img_y;
                wxPoint mouse_point = wxGetMousePosition();
                mouse_point = this->ScreenToClient(mouse_point);
                screenToImage(mouse_point.x, mouse_point.y, mouse_img_x, mouse_img_y);
                mouse_img_x -= size / 2;
                mouse_img_y -= size / 2;
                int mouse_scr_x, mouse_scr_y;
                imageToScreen(mouse_img_x, mouse_img_y, mouse_scr_x, mouse_scr_y);
                gc->SetPen(wxPen(wxColour(0, 255, 0), 1));
                gc->DrawRectangle(mouse_scr_x, mouse_scr_y, size * scale, size * scale);
            }
        }

        delete gc;
    }
    void onSize()
    {
        Refresh();
    }
    wxSize DoGetBestSize() const override
    {
        wxSize size = GetSize();
        size.SetHeight(max(size.GetHeight(), 100));
        size.SetWidth(max(size.GetWidth(), 100));
        return size;
    }
    virtual bool AcceptsFocus() const
    {
        return true;
    }
    void sendStartPaintEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_CANVAS_PAINT, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
};

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
    }
    int getSelection()
    {
        return selection;
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
wxDECLARE_EVENT(EVT_TOOL_OPTION_CHANGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_TOOL_OPTION_CHANGE, wxCommandEvent);

class ToolOptionSizeCtrl : public wxPanel
{
public:
    ToolOptionSizeCtrl(wxWindow *parent, wxWindowID id = wxID_ANY) : wxPanel(parent, id)
    {
        wxBoxSizer *box = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Size:"));
        box->Add(label, 0, wxEXPAND);
        spin = new wxSpinCtrl(this, wxID_ANY);
        spin->SetMin(1);
        spin->SetMax(16);
        box->Add(spin, 1, wxEXPAND);
        Bind(wxEVT_SPINCTRL, &ToolOptionSizeCtrl::sendToolOptionChangeEvent, this);
        SetSizer(box);
    }
    void setValue(int value)
    {
        spin->SetValue(value);
    }
    int getValue()
    {
        return spin->GetValue();
    }

protected:
    wxSpinCtrl *spin;
    void sendToolOptionChangeEvent(wxCommandEvent &e)
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_TOOL_OPTION_CHANGE, GetId());
        event->SetEventObject(this);
        event->SetString("SIZE");
        event->SetInt(spin->GetValue());
        wxQueueEvent(GetEventHandler(), event);
    }
};

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
    static const int tool_count = 10;
    ToolBox(wxWindow *parent, wxWindowID id = wxID_ANY) : wxSplitterWindow(parent, id)
    {
        this->SetWindowStyle(wxSP_3DSASH | wxSP_LIVE_UPDATE | wxSP_NO_XP_THEME);
        button_list = new ToolButtonList(this);
        for (int i = 0; i < tool_count; i++)
        {
            wxBitmapToggleButton *button = new wxBitmapToggleButton(button_list, wxID_ANY, wxBitmap(10, 10));
            button_list->addButton(button);
        }

        option_book = new wxSimplebook(this, wxID_ANY);
        for (int i = 0; i < tool_count; i++)
        {
            wxScrolledWindow *panel = new wxScrolledWindow(option_book);
            panel->SetScrollRate(0, 5);
            ToolOptionSizeCtrl *size_ctrl = new ToolOptionSizeCtrl(panel);
            wxGridSizer *grid = new wxGridSizer(2);
            grid->Add(size_ctrl, 0, wxALL, 2);
            panel->SetSizer(grid);
            size_ctrls.push_back(size_ctrl);

            // add special tool ctrls.
            switch (i)
            {
            case 0:
                /* code */
                break;

            default:
                break;
            }

            option_book->AddPage(panel, wxString::FromUTF8("Tool"));
        }

        this->SplitVertically(button_list, option_book, 100);

        Bind(EVT_BUTTON_LIST_CHANGED, &ToolBox::onButtonChange, this);
        Bind(EVT_TOOL_OPTION_CHANGE, &ToolBox::onOptionChange, this);

        // create new tool objects
        tool_list.push_back(new SolidPen());
        tool_list.push_back(new NoisePen());
        tool_list.push_back(new Eraser());
    }
    ~ToolBox()
    {
        for (auto i = tool_list.begin(); i != tool_list.end(); i++)
        {
            delete *i;
        }
    }
    /**
     * Get the active tool. Dont delete the returned object.
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

protected:
    ToolButtonList *button_list;
    vector<Tool *> tool_list;
    wxSimplebook *option_book;
    vector<ToolOptionSizeCtrl *> size_ctrls;
    bool independent_size = false;

    void onOptionChange(wxCommandEvent &event)
    {
        // process the options;
        if (!independent_size && event.GetString() == "SIZE")
        {
            for (auto i = size_ctrls.begin(); i != size_ctrls.end(); i++)
            {
                (*i)->setValue(event.GetInt());
            }
            for (auto i = tool_list.begin(); i != tool_list.end(); i++)
            {
                (*i)->setProperty("SIZE", event.GetInt());
            }
        }
    }
    void onButtonChange(wxCommandEvent &event)
    {
        option_book->ChangeSelection(button_list->getSelection());
        sendToolChangeEvent();
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

class MyFrame : public wxFrame
{
public:
    MyFrame() : wxFrame(NULL, wxID_ANY, "Hello World")
    {
        CommandManager::create();
        makeMenu();
        wxInitAllImageHandlers();

        mainP = new wxPanel(this);
        wxSplitterWindow *splitter = new wxSplitterWindow(mainP);
        viewer = new LayerViewer(splitter);
        Bind(EVT_LAYER_CHANGE, &MyFrame::onLayerChange, this, viewer->GetId());

        lcp = new LayerControlPanel(splitter);
        Bind(EVT_LAYER_RENAME, &MyFrame::onLayerRename, this, lcp->GetId());
        splitter->SplitHorizontally(viewer, lcp, viewer->GetBestHeight(100));
        splitter->SetWindowStyle(wxSP_LIVE_UPDATE | wxSP_3D | wxSP_NO_XP_THEME);

        myskin = new Skin("my skin");
        myskin->addLayer(new Layer("Test Layer", myskin->getLayerSize()));
        viewer->loadSkin(myskin);

        wxBoxSizer *box = new wxBoxSizer(wxHORIZONTAL);
        color_picker = new AdvColorPicker(mainP);
        box->Add(color_picker, 1, wxEXPAND | wxALL, 5);
        Bind(EVT_COLOR_PICKER_CHANGE, &MyFrame::onColorChange, this, color_picker->GetId());

        wxSplitterWindow *middle_splitter = new wxSplitterWindow(mainP);

        canvas = new Canvas(middle_splitter);
        canvas->loadSkin(myskin);

        tool_box = new ToolBox(middle_splitter);
        Bind(EVT_TOOL_CHANGE, &MyFrame::onToolChange, this, tool_box->GetId());

        middle_splitter->SetWindowStyle(wxSP_LIVE_UPDATE | wxSP_3D | wxSP_NO_XP_THEME);
        middle_splitter->SplitHorizontally(tool_box, canvas, 40);

        box->Add(middle_splitter, 2, wxEXPAND | wxALL, 5);
        box->Add(splitter, 1, wxEXPAND | wxALL, 5);

        Bind(EVT_LAYER_UPDATE, &MyFrame::onNeedRefresh, this);

        mainP->SetSizer(box);
        this->SetSize(1000, 600);
    }
    ~MyFrame()
    {
        delete myskin;
        CommandManager::destruct();
    }

    void makeMenu()
    {
        wxMenuBar *bar = new wxMenuBar();
        this->SetMenuBar(bar);

        wxMenu *menu_file = new wxMenu();
        wxMenu *menu_edit = new wxMenu();
        wxMenu *menu_view = new wxMenu();

        bar->Append(menu_file, _T("File"));
        bar->Append(menu_edit, _T("Edit"));
        bar->Append(menu_view, _T("View"));

        wxMenuItem *item;
        item = menu_file->Append(wxID_ANY, _T("New\tCtrl+N"), _T("create a new skin file"));
        Bind(wxEVT_MENU, &MyFrame::onNewSkin, this, item->GetId());

        item = menu_file->Append(wxID_ANY, _T("Open\tCtrl+O"), _T("open a skin file"));
        item = menu_file->Append(wxID_ANY, _T("Save\tCtrl+S"), _T("save to a skin file"));
        item = menu_file->Append(wxID_ANY, _T("Import"), _T("Import a png file to a layer"));
        Bind(wxEVT_MENU, &MyFrame::onImport, this, item->GetId());
        item = menu_file->Append(wxID_ANY, _T("Export"), _T("export to a png file"));
        Bind(wxEVT_MENU, &MyFrame::onExport, this, item->GetId());
        item = menu_file->Append(wxID_ANY, _T("Close"), _T("close the current file"));

        item = menu_edit->Append(wxID_ANY, _T("Redo\tCtrl+Shift+Z"), _T("redo last operation"));
        Bind(wxEVT_MENU, &MyFrame::onRedo, this, item->GetId());
        item = menu_edit->Append(wxID_ANY, _T("Undo\tCtrl+Z"), _T("undo last operation"));
        Bind(wxEVT_MENU, &MyFrame::onUndo, this, item->GetId());

        item = menu_view->Append(wxID_ANY, _T("Open Reference Image"), _T("open a reference image"));
        Bind(wxEVT_MENU, &MyFrame::onOpenReference, this, item->GetId());
    }

    wxPanel *mainP;
    LayerViewer *viewer;
    LayerControlPanel *lcp;
    ToolBox *tool_box;
    Canvas *canvas;
    AdvColorPicker *color_picker;

protected:
    Skin *myskin = nullptr;
    void onToolChange(wxCommandEvent &event)
    {
        canvas->setPen(tool_box->getTool());
    }
    void onLayerRename(wxCommandEvent &event)
    {
        viewer->refreshNames();
    }
    void onLayerChange(wxCommandEvent &event)
    {
        if (myskin->getLayerNum() != 0)
        {
            lcp->loadLayer(myskin->getLayer(event.GetInt()));
        }
        else
        {
            lcp->clear();
        }
        canvas->loadLayer(event.GetInt());
    }
    void onNeedRefresh(wxCommandEvent &event)
    {
        canvas->Refresh();
        canvas->Update();
    }
    void onColorChange(wxCommandEvent &event)
    {
        ColorPicker *picker = dynamic_cast<ColorPicker *>(event.GetEventObject());
        if (picker != nullptr)
        {
            Color *color = picker->getColor();
            tool_box->setColor(color);
            if(picker != color_picker){
                color_picker->setColor(*color);
            }
            delete color;
        }
    }

    void onNewSkin(wxCommandEvent &event)
    {
        if (myskin != nullptr)
        {
            // some thing is opened
            int ret = wxMessageBox(_T("Save current skin?"), _T("Unsaved skin"), wxYES_NO | wxCANCEL);
            if (ret == wxOK)
            {
                // save
                wxMessageBox(_T("Sorry! Saving is not support yet.."));
                return;
            }
            else if (ret == wxCANCEL)
            {
                // canceled
                return;
            }
            else
            {
                // don't save
                delete myskin;
                myskin = nullptr;
            }
            canvas->loadSkin(nullptr);
            canvas->loadLayer(-1);
            viewer->clear();
            lcp->clear();
        }
        // create new skin
        wxArrayString choices;
        choices.Add(_T("Steve"));
        choices.Add(_T("Alex"));
        choices.Add(_T("Steve min"));
        choices.Add(_T("Alex min"));
        int answer = wxGetSingleChoiceIndex(_T("Choose a skin format"),
                                            _T("new skin"), choices, 0, this);
        switch (answer)
        {
        case 0:
            myskin = new Skin("new skin", SkinType::STEVE);
            break;
        case 1:
            myskin = new Skin("new skin", SkinType::ALEX);
            break;
        case 2:
            myskin = new Skin("new skin", SkinType::STEVE_MIN);
            break;
        case 3:
            myskin = new Skin("new skin", SkinType::ALEX_MIN);
            break;
        default:
            break;
        }
        canvas->loadSkin(myskin);
        viewer->loadSkin(myskin);
    }
    void onExport(wxCommandEvent &event)
    {
        wxFileDialog *export_dialog = new wxFileDialog(this, _T("Export png"),
                                                       wxEmptyString, wxEmptyString, _T("PNG File|*.png"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        int ret = export_dialog->ShowModal();
        if (ret == wxID_OK)
        {
            wxString path = export_dialog->GetPath();
            wxImage image = this->myskin->render();
            image.SaveFile(path, wxBITMAP_TYPE_PNG);
        }
        export_dialog->Destroy();
    }
    void onImport(wxCommandEvent &event)
    {
        wxFileDialog *import_dialog = new wxFileDialog(this, _T("Import png"),
                                                       wxEmptyString, wxEmptyString, _T("PNG File|*.png"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        int ret = import_dialog->ShowModal();
        if (ret == wxID_OK)
        {
            wxString path = import_dialog->GetPath();
            wxImage image;
            image.LoadFile(path);
            wxSize skin_size = myskin->getLayerSize();
            image.Rescale(skin_size.x, skin_size.y);
            if (!image.HasAlpha())
            {
                image.InitAlpha();
            }
            Layer *new_layer = new Layer("Imported Layer", myskin->getLayerSize());
            new_layer->getImage()->Paste(image, 0, 0);
            myskin->addLayer(new_layer);
            viewer->loadSkin(myskin);
        }

        import_dialog->Destroy();
        canvas->Refresh();
        canvas->Update();
    }

    void onUndo(wxCommandEvent &event)
    {
        CommandManager::getInstance()->undo();
        canvas->Refresh();
        canvas->Update();
    }
    void onRedo(wxCommandEvent &event)
    {
        CommandManager::getInstance()->redo();
        canvas->Refresh();
        canvas->Update();
    }

    void onOpenReference(wxCommandEvent &event)
    {
        wxFileDialog* fd = new wxFileDialog(this, _T("Choose a reference image"),
                                       wxEmptyString, wxEmptyString,
                                       _T("All Supported Images|*.bmp;*.jpg;*.png;*.gif;*.tif;*.tiff|All Files|*.*"),
                                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        int ret = fd->ShowModal();
        if (ret == wxID_OK)
        {
            // opened a file
            ReferenceWindow *window = new ReferenceWindow(this);
            window->loadImage(fd->GetPath());
            Bind(EVT_COLOR_PICKER_CHANGE, &MyFrame::onColorChange, this, window->GetId());
            window->Show();
        }
        fd->Destroy();
    }
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit()
    {
        MyFrame *frame = new MyFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
