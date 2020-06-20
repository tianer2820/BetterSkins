#if !defined(REFERENCE_WINDOW_H)
#define REFERENCE_WINDOW_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/frame.h>

#include "colorPickers/colorPicker.h"

class ReferenceWindow : public wxFrame, public ColorPicker
{
public:
    ReferenceWindow(wxWindow *parent, wxWindowID id = wxID_ANY) : wxFrame(parent,
                                                                          id,
                                                                          _T("Reference"), wxDefaultPosition,
                                                                          wxDefaultSize,
                                                                          wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT)
    {
        // this->SetWindowStyleFlag();
        this->SetBackgroundStyle(wxBG_STYLE_PAINT);

        Bind(wxEVT_PAINT, &ReferenceWindow::onPaint, this);
        Bind(wxEVT_MOTION, &ReferenceWindow::onMouse, this);
        Bind(wxEVT_MOUSEWHEEL, &ReferenceWindow::onMouse, this);
        Bind(wxEVT_MIDDLE_DOWN, &ReferenceWindow::onMouse, this);
        Bind(wxEVT_MIDDLE_UP, &ReferenceWindow::onMouse, this);
        Bind(wxEVT_LEFT_DOWN, &ReferenceWindow::onMouse, this);
        Bind(wxEVT_LEFT_UP, &ReferenceWindow::onMouse, this);
    }

    /**
     * return true if successful, other wise false.
     * load a image file into this window.
     */
    bool loadImage(wxString file_name)
    {
        bool ret = img.LoadFile(file_name);
        Refresh();
        return ret;
    }

    /**
     * you need to delete the returned object after use!
     */
    virtual Color *getColor()
    {
        return current_color.copy();
    }
    virtual void setColor(Color &color)
    {
        return;
    }

protected:
    wxImage img;
    int x_offset = 0;
    int y_offset = 0;
    double scale = 1;

    double drag_offset_x;
    double drag_offset_y;

    bool dragging = false;
    bool picking = false;

    RGBColor current_color;

    void screenToImage(int x, int y, int &out_x, int &out_y)
    {
        out_x = (x - x_offset) / scale;
        out_y = (y - y_offset) / scale;
    }
    void onMouse(wxMouseEvent &event)
    {
        event.Skip();
        int x = event.GetX();
        int y = event.GetY();

        bool doing_operations = dragging || picking;
        if (!doing_operations)
        {
            if (event.LeftDown())
            {
                //start picking
                picking = true;
                CaptureMouse();
            }
            else if (event.MiddleDown())
            {
                //start dragging
                dragging = true;
                CaptureMouse();
                drag_offset_x = (x - x_offset) / scale;
                drag_offset_y = (y - y_offset) / scale;
            }
        }
        int wheel_delta = event.GetWheelDelta();
        if (event.GetWheelRotation() != 0)
        {
            double rotation = event.GetWheelRotation() / wheel_delta;
            scale *= pow(1.2, rotation);
            x_offset -= (x - x_offset) * (pow(1.2, rotation) - 1);
            y_offset -= (y - y_offset) * (pow(1.2, rotation) - 1);
            Refresh();
        }

        if (picking)
        {
            int img_x, img_y;
            screenToImage(event.GetX(), event.GetY(), img_x, img_y);
            u_char *data = img.GetData();
            int color[3];
            int index = (img_y * img.GetWidth() + img_x) * 3;
            for (int i = 0; i < 3; i++)
            {
                color[i] = data[index + i];
            }
            current_color.setRGB(color);
            sendColorChangeEvent();
            if (event.LeftUp())
            {
                picking = false;
                ReleaseMouse();
            }
        }
        if (dragging)
        {
            x_offset = x - drag_offset_x * scale;
            y_offset = y - drag_offset_y * scale;
            Refresh();
            if (event.MiddleUp())
            {
                dragging = false;
                ReleaseMouse();
            }
        }
    }
    void onPaint(wxPaintEvent &event)
    {
        event.Skip();
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        if (!img.IsOk())
        {
            return;
        }
        // if the image is good to draw:
        wxImage screen = wxImage(this->GetClientSize());
        screen.InitAlpha();
        u_char *data = screen.GetData();
        u_char *alpha = screen.GetAlpha();
        int w = screen.GetWidth();
        int h = screen.GetHeight();

        u_char *img_data = img.GetData();
        int img_w = img.GetWidth();
        int img_h = img.GetHeight();

        wxColour bg_color = this->GetBackgroundColour();
        int c[3] = {bg_color.Red(), bg_color.Green(), bg_color.Blue()};

        if (scale > 1) // scale up, raw pixels are shown
        {
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    int img_x, img_y;
                    screenToImage(x, y, img_x, img_y);
                    int index = (y * w + x);

                    if (img_x < 0 || img_y < 0 || img_x >= img_w || img_y >= img_h)
                    {
                        // set alpha transparent
                        alpha[index] = 0;
                    }
                    else
                    {
                        int img_i = (img_y * img_w + img_x);
                        for (int i = 0; i < 3; i++) // fill pixel
                        {
                            data[index * 3 + i] = img_data[img_i * 3 + i];
                        }
                        alpha[index] = img.GetAlpha(img_x, img_y);
                    }
                }
            }
        }
        else
        { // scale down, use anti-aliasing
            // fill bg color
            for (int i = 0; i < w * h; i++)
            {
                for (int channel = 0; channel < 3; channel++)
                {
                    data[i*3 + channel] = c[channel];
                }
            }
            
            wxImage scaled = img.Scale(img_w * scale, img_h * scale, wxIMAGE_QUALITY_BILINEAR);
            screen.Paste(scaled, x_offset, y_offset);
        }

        dc.DrawBitmap(wxBitmap(screen), 0, 0);
    }
    void sendColorChangeEvent()
    {
        wxCommandEvent *event = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, this->GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }
};

#endif // REFERENCE_WINDOW_H
