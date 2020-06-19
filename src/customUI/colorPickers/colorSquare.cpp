
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "../../color/color.h"
#include "colorPicker.h"
#include "colorSquare.h"


    ColorSquare::ColorSquare(wxWindow *parent, wxWindowID id) : wxWindow(parent, id)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_SIZE, &ColorSquare::onSize, this);
        Bind(wxEVT_PAINT, &ColorSquare::onPaint, this);

        Bind(wxEVT_LEFT_DOWN, &ColorSquare::onMouse, this);
        Bind(wxEVT_MOTION, &ColorSquare::onMouse, this);
        Bind(wxEVT_LEFT_UP, &ColorSquare::onMouse, this);
    }

    Color *ColorSquare::getColor()
    {
        return current_color.copy();
    }

    void ColorSquare::setColor(Color &color)
    {
        int hsv[3];
        color.getHSV(hsv);
        current_color.setHSV(hsv);
        need_redraw_square = true;
        Refresh();
        Update();
    }

    void ColorSquare::onMouse(wxMouseEvent &event)
    {
        event.Skip();
        int width = current_image.GetWidth();
        int height = current_image.GetHeight();
        int square_side;
        int bar_width;
        if (width * 1.0 / 11 < height * 1.0 / 10)
        {
            // width is smaller
            square_side = width * 10 / 11;
            bar_width = square_side / 10;
        }
        else
        {
            // height is smaller
            square_side = height;
            bar_width = square_side / 10;
        }
        int total_width = square_side + bar_width;
        int offset_x = (width - total_width) / 2;
        int offset_y = (height - square_side) / 2;

        int x = event.GetX();
        int y = event.GetY();

        if(event.LeftDown()){
            // start dragging?
            if (x >= offset_x && x < offset_x + square_side && y >= offset_y && y < offset_y + square_side)
            {
                // if in square
                CaptureMouse();
                dragging_square = true;
            }
            else if (x >= offset_x + square_side && x < offset_x + square_side + bar_width && y >= offset_y && y < offset_y + square_side)
            {
                // if in bar
                CaptureMouse();
                dragging_bar = true;
            }
            else
            {
                // if in background do nothing
            }
        } else if (event.LeftUp() && (dragging_bar || dragging_square)){
            ReleaseMouse();
            dragging_bar = false;
            dragging_square = false;
        }

        if(dragging_bar){
            int hsv[3];
            current_color.getHSV(hsv);
            hsv[0] = min(max((y - offset_y) * 360 / square_side ,0), 360);
            
            current_color.setHSV(hsv);
            need_redraw_square = true;
            Refresh();
            Update();
            sendColorPickerUpdateEvent();
        } else if (dragging_square){
            int hsv[3];
            current_color.getHSV(hsv);
            hsv[1] = min(255, max(0, (x - offset_x) * 255 / square_side));
            hsv[2] = min(255, max(0, 255 - (y - offset_y) * 255 / square_side));
            current_color.setHSV(hsv);
            Refresh();
            Update();
            sendColorPickerUpdateEvent();
        }
    }

    void ColorSquare::onSize(wxSizeEvent &event)
    {
        Refresh();
        need_redraw_bar = true;
        need_redraw_square = true;
        wxSize size = this->GetSize();
        if (size.x < 1)
        {
            size.x = 1;
        }
        if (size.y < 1)
        {
            size.y = 1;
        }
        current_image = wxImage(size);
    }

    void ColorSquare::onPaint(wxPaintEvent &event)
    {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        if (need_redraw_bar || need_redraw_square)
        {
            drawImage();
        }
        dc.DrawBitmap(wxBitmap(current_image), 0, 0);
        int hsv[3];
        current_color.getHSV(hsv);
        int width = current_image.GetWidth();
        int height = current_image.GetHeight();
        int square_side;
        int bar_width;
        if (width * 1.0 / 11 < height * 1.0 / 10)
        {
            // width is smaller
            square_side = width * 10 / 11;
            bar_width = square_side / 10;
        }
        else
        {
            // height is smaller
            square_side = height;
            bar_width = square_side / 10;
        }
        int total_width = square_side + bar_width;
        int offset_x = (width - total_width) / 2;
        int offset_y = (height - square_side) / 2;

        double x = hsv[1] * square_side / 255 + offset_x;
        double y = square_side - hsv[2] * square_side / 255 + offset_y;
        double bar_x = square_side + offset_x + bar_width / 2;
        double bar_y = hsv[0] * square_side / 360 + offset_y;
        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
        wxGraphicsPath path = gc->CreatePath();

        gc->SetPen(wxPen(wxColor(0, 0, 0), 1));
        // draw circle
        path.AddCircle(x, y, bar_width / 2);
        // draw bar
        path.AddCircle(bar_x, bar_y, bar_width / 2);
        path.MoveToPoint(square_side + offset_x, bar_y);
        path.AddLineToPoint(square_side + offset_x + bar_width, bar_y);
        gc->StrokePath(path);

        path = gc->CreatePath();
        gc->SetPen(wxPen(wxColor(255, 255, 255), 1));
        // draw circle
        path.AddCircle(x, y, bar_width / 2 - 1);
        // draw bar
        path.AddCircle(bar_x, bar_y, bar_width / 2 - 1);
        path.MoveToPoint(square_side + offset_x, bar_y + 1);
        path.AddLineToPoint(square_side + offset_x + bar_width, bar_y + 1);
        path.MoveToPoint(square_side + offset_x, bar_y - 1);
        path.AddLineToPoint(square_side + offset_x + bar_width, bar_y - 1);
        gc->StrokePath(path);

        delete gc;
    }

    void ColorSquare::drawImage()
    {
        u_char *data = current_image.GetData();
        int width = current_image.GetWidth();
        int height = current_image.GetHeight();
        int square_side;
        int bar_width;
        if (width * 1.0 / 11 < height * 1.0 / 10)
        {
            // width is smaller
            square_side = width * 10 / 11;
            bar_width = square_side / 10;
        }
        else
        {
            // height is smaller
            square_side = height;
            bar_width = square_side / 10;
        }
        int total_width = square_side + bar_width;
        int offset_x = (width - total_width) / 2;
        int offset_y = (height - square_side) / 2;

        int i = 0;
        int hsv[3];
        current_color.getHSV(hsv);

        wxColor bg = GetBackgroundColour();
        int bg_color[3] = {bg.Red(), bg.Green(), bg.Blue()};

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if (x >= offset_x && x < offset_x + square_side && y >= offset_y && y < offset_y + square_side)
                {
                    // if in square
                    if (need_redraw_square)
                    {
                        int temp[3] = {hsv[0], (x - offset_x) * 255 / square_side, 255 - (y - offset_y) * 255 / square_side};
                        int rgb[3];
                        Color::HSV2RGB(temp, rgb);
                        for (int j = 0; j < 3; j++)
                        {
                            data[i + j] = rgb[j];
                        }
                    }
                }
                else if (x >= offset_x + square_side && x < offset_x + square_side + bar_width && y >= offset_y && y < offset_y + square_side)
                {
                    // if in bar
                    int temp[3] = {(y - offset_y) * 360 / square_side, 255, 255};
                    int rgb[3];
                    Color::HSV2RGB(temp, rgb);
                    for (int j = 0; j < 3; j++)
                    {
                        data[i + j] = rgb[j];
                    }
                }
                else
                {
                    // if in background
                    for (int j = 0; j < 3; j++)
                    {
                        data[i + j] = bg_color[j];
                    }
                }
                i += 3;
            }
        }

        need_redraw_bar = false;
        need_redraw_square = false;
    }

    void ColorSquare::sendColorPickerUpdateEvent(){
        wxCommandEvent* event = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, GetId());
        event->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), event);
    }

    wxSize ColorSquare::DoGetBestSize() const
    {
        return wxSize(50, 50);
    }

