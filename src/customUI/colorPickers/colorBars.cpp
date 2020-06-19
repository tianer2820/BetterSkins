

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "../../color/color.h"
#include "colorBars.h"

wxDEFINE_EVENT(EVT_COLOR_BAR_CHANGE, wxCommandEvent);

ColorBar::ColorBar(wxWindow *parent, wxWindowID id, int gradiaentChannel) : wxWindow(parent, id),
                                                                            channel(gradiaentChannel),
                                                                            img(1, 1)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &ColorBar::onPaint, this);
    Bind(wxEVT_SIZE, &ColorBar::onSize, this);

    Bind(wxEVT_LEFT_DOWN, &ColorBar::onMouse, this);
    Bind(wxEVT_LEFT_UP, &ColorBar::onMouse, this);
    Bind(wxEVT_MOTION, &ColorBar::onMouse, this);
    Bind(wxEVT_ENTER_WINDOW, &ColorBar::onMouse, this);
    Bind(wxEVT_LEAVE_WINDOW, &ColorBar::onMouse, this);
    Bind(wxEVT_MOUSEWHEEL, &ColorBar::onMouse, this);
}
void ColorBar::setChannel(int channel, int value)
{
    color[channel] = max(0, min(channelMax, value));
    needRedraw = true;
    Refresh();
    Update();
}
void ColorBar::setValue(int value)
{
    color[channel] = max(0, min(channelMax, value));
    Refresh();
    Update();
}
void ColorBar::setColor(int *color)
{
    for (int i = 0; i < 3; i += 1)
    {
        this->color[i] = color[i];
    }
    needRedraw = true;
    Refresh();
    Update();
}
int ColorBar::getValue()
{
    return color[channel];
}

void ColorBar::onSize(wxSizeEvent &event)
{
    needRedraw = true;
    Refresh();
    event.Skip();
}
void ColorBar::onMouse(wxMouseEvent &event)
{
    event.Skip();
    if (event.LeftDown())
    {
        //start drag
        this->CaptureMouse();
        dragging = true;
    }
    if (dragging)
    {
        int w = GetSize().GetWidth();
        int x = max(0, min(w, event.GetX()));
        color[channel] = double(x) / w * channelMax;
        if (event.LeftUp())
        {
            this->ReleaseMouse();
            dragging = false;
        }
        sendValueChangeEvent();
        Refresh();
        Update();
    }
    else if (event.GetWheelRotation() != 0)
    {
        color[channel] += event.GetWheelRotation() / event.GetWheelDelta();
        color[channel] = max(0, min(channelMax, color[channel]));
        sendValueChangeEvent();
        Refresh();
        Update();
    }
}
void ColorBar::sendValueChangeEvent()
{
    wxCommandEvent *event = new wxCommandEvent(EVT_COLOR_BAR_CHANGE, GetId());
    event->SetEventObject(this);
    event->SetInt(this->color[channel]);
    wxQueueEvent(GetEventHandler(), event);
}
void ColorBar::onPaint(wxPaintEvent &event)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxSize size = GetSize();
    int w = max(1, size.GetWidth());
    int h = max(1, size.GetHeight());

    if (needRedraw)
    {
        if (size != img.GetSize())
        {
            //size changed
            img = wxImage(w, h);
        }
        this->drawImg(img);
        needRedraw = false;
    }
    dc.DrawBitmap(wxBitmap(img), 0, 0);
    double lineX = double(color[channel]) / channelMax * w;

    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
    gc->SetPen(wxPen(wxColor(0, 0, 0), 1, wxPENSTYLE_SOLID));
    wxGraphicsPath path = gc->CreatePath();
    path.AddRectangle(0, 0, w - 1, h - 1);
    path.MoveToPoint(lineX + 1, 0);
    path.AddLineToPoint(lineX + 1, h);
    path.MoveToPoint(lineX - 1, 0);
    path.AddLineToPoint(lineX - 1, h);
    path.AddCircle(lineX, h / 2, h / 4);
    gc->StrokePath(path);

    gc->SetPen(wxPen(wxColor(255, 255, 255), 1, wxPENSTYLE_SOLID));
    path = gc->CreatePath();
    path.MoveToPoint(lineX, 0);
    path.AddLineToPoint(lineX, h);
    path.AddCircle(lineX, h / 2, h / 4 - 1);
    gc->StrokePath(path);
    delete gc;
}
void ColorBar::drawImg(wxImage &image)
{
    wxSize size = image.GetSize();
    int w = size.GetWidth();
    int h = size.GetHeight();
    int currentColor[3];
    int rgb[3];
    u_char *data = image.GetData();

    for (int i = 0; i < 3; i += 1)
    {
        currentColor[i] = color[i];
    }
    for (int x = 0; x < w; x += 1)
    {
        currentColor[channel] = double(x) / w * channelMax;
        this->colorSpaceToRGB(currentColor, rgb);
        for (int y = 0; y < h; y += 1)
        {
            int i = (y * w + x) * 3;
            data[i] = rgb[0];
            data[i + 1] = rgb[1];
            data[i + 2] = rgb[2];
        }
    }
}

wxSize ColorBar::DoGetBestSize() const
{
    return wxSize(100, 20);
}

RGBBar::RGBBar(wxWindow *parent, wxWindowID id, int gradiaentChannel) : ColorBar(parent, id, gradiaentChannel)
{
}

void RGBBar::colorSpaceToRGB(int *color, int *out)
{
    for (int i = 0; i < 3; i++)
    {
        out[i] = color[i];
    }
}

HSVBar::HSVBar(wxWindow *parent, wxWindowID id, int gradiaentChannel) : ColorBar(parent, id, gradiaentChannel)
{
    if (channel == 0)
    {
        channelMax = 359;
    }
}

void HSVBar::colorSpaceToRGB(int *color, int *out)
{
    int out2[3];
    RGBColor::HSV2RGB(color, out2);
    for (int i = 0; i < 3; i++)
    {
        out[i] = out2[i];
    }
}
