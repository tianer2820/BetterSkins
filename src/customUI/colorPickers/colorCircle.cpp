#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#include "../../color/color.h"
#include "colorCircle.h"

using namespace std;


ColorCircle::ColorCircle(wxWindow *parent, wxWindowID id) : wxPanel(parent, id), last_image(1, 1)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &ColorCircle::onPaint, this);
    Bind(wxEVT_SIZE, &ColorCircle::onSize, this);

    Bind(wxEVT_LEFT_DOWN, &ColorCircle::onMouse, this);
    Bind(wxEVT_LEFT_UP, &ColorCircle::onMouse, this);
    Bind(wxEVT_MOTION, &ColorCircle::onMouse, this);
    Bind(wxEVT_ENTER_WINDOW, &ColorCircle::onMouse, this);
    Bind(wxEVT_LEAVE_WINDOW, &ColorCircle::onMouse, this);
}
void ColorCircle::setColor(Color& newColor)
{
    newColor.getHSV(hsv);
    need_redraw_triangle = true;
    this->Refresh();
    Update();
}

// you have to delete the returned object manualy!!!
Color* ColorCircle::getColor()
{
    Color* out = new HSVColor();
    out->setHSV(hsv);
    return out;
}
wxSize ColorCircle::DoGetBestSize() const
{
    wxSize size = GetSize();
    if (size.GetHeight() < 50)
    {
        size.SetHeight(50);
    }
    else if (size.GetWidth() < 50)
    {
        size.SetWidth(50);
    }
    return size;
}
void ColorCircle::onSize(wxSizeEvent &event)
{
    need_redraw_circle = true;
    need_redraw_triangle = true;
    this->Refresh();
    event.Skip();
}
void ColorCircle::onPaint(wxPaintEvent &event)
{
    wxSize size = GetSize();
    int w = max(1, size.GetWidth());
    int h = max(1, size.GetHeight());
    int cw = w / 2;
    int ch = h / 2;
    int r = min(cw, ch) - 2;
    int bandWidth = r * 0.2;

    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    if (!need_redraw_circle)
    {
        if (need_redraw_triangle)
        {
            // update triangle only
            paintImage(last_image);
            need_redraw_circle = false;
            need_redraw_triangle = false;
        }
        else
        {
            // update nothing
            need_redraw_circle = false;
            need_redraw_triangle = false;
        }
    }
    else
    {
        // update both
        wxImage bufferImage(w, h);
        paintImage(bufferImage);
        last_image = bufferImage;
        need_redraw_circle = false;
        need_redraw_triangle = false;
    }

    //draw circle triangle and bar
    double hAngle = (hsv[0] - 180) * 3.14 / 180;
    double hs = sin(hAngle);
    double hc = cos(hAngle);
    double bx1 = r * hc + cw;
    double by1 = r * hs + ch;
    double bx2 = bx1 - bandWidth * hc;
    double by2 = by1 - bandWidth * hs;

    double triTop = ch - (r - bandWidth);
    double triBottom = ch + sin(3.1415 / 6) * (r - bandWidth);
    double triHeight = triBottom - triTop;
    double triWidth = sin(3.1415 / 3) * (r - bandWidth);
    double limL = cw - triWidth;
    double limR = cw + triWidth;

    double pointY = double(hsv[2]) / 255 * triHeight + triTop;
    double pointX = double(hsv[1]) / 255 * 2 * triWidth * (double(hsv[2]) / 255) + limL + triWidth * (1 - double(hsv[2]) / 255);

    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
    gc->SetPen(wxPen(wxColor(0, 0, 0), 1, wxPENSTYLE_SOLID));
    gc->SetBrush(wxBrush(wxBitmap(last_image)));

    wxGraphicsPath path = gc->CreatePath();
    // hue circles
    path.AddCircle(cw, ch, r - 1);
    path.AddCircle(cw, ch, r - bandWidth + 1);

    // triangle
    path.MoveToPoint(cw, triTop + 1);
    path.AddLineToPoint(limL + 1, triBottom);
    path.AddLineToPoint(limR - 1, triBottom);
    path.AddLineToPoint(cw, triTop);
    // fill the color
    gc->FillPath(path);
    
    path = gc->CreatePath();
    // draw the bar and point circles
    path.MoveToPoint(bx1, by1);
    path.AddLineToPoint(bx2, by2);

    // sv circle 1
    path.AddCircle(pointX, pointY, bandWidth / 3);
    gc->StrokePath(path);
    

    // reset pen
    gc->SetPen(wxPen(wxColor(255, 255, 255), 1, wxPENSTYLE_SOLID));
    path = gc->CreatePath();
    // sv circle 2;
    path.AddCircle(pointX, pointY, bandWidth / 3 - 1);
    gc->StrokePath(path);
    // push the result
    delete gc;
}
void ColorCircle::onMouse(wxMouseEvent &event)
{
    event.Skip();
    int x = event.GetX();
    int y = event.GetY();
    wxSize size = GetSize();
    int w = size.GetWidth();
    int h = size.GetHeight();
    int cw = w / 2;
    int ch = h / 2;
    int area = this->checkArea(x, y);
    if (area == 0)
    {
        // not in area
    }
    else if (area == 1)
    {
        // mouse on circle
        if (event.LeftDown())
        {
            //start drag
            this->CaptureMouse();
            draging_circle = true;
        }
    }
    else if (area == 2)
    {
        // mouse on triangle
        if (event.LeftDown())
        {
            //start drag
            this->CaptureMouse();
            draging_triangle = true;
        }
    }
    if (draging_circle)
    {
        int angle = atan2(y - ch, x - cw) / 3.1415 * 180 + 180;
        hsv[0] = angle;

        wxCommandEvent *e = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, GetId());
        e->SetEventObject(this);
        wxQueueEvent(this->GetEventHandler(), e);

        need_redraw_triangle = true;
        Refresh();
        Update();

        if (event.LeftUp())
        {
            // finish drag
            this->ReleaseMouse();
            draging_circle = false;
        }
    }
    if (draging_triangle)
    {
        int r = min(w, h) / 2 - 2;
        int bandWidth = r * 0.2;
        int triTop = ch - (r - bandWidth);
        int triBottom = ch + sin(3.1415 / 6) * (r - bandWidth);
        y = max(min(y, triBottom), triTop);
        int triHeight = triBottom - triTop;
        int triWidth = sin(3.1415 / 3) * (r - bandWidth) * (double(y - triTop) / triHeight);
        int limL = cw - triWidth;
        int limR = cw + triWidth;
        x = max(min(x, limR), limL);
        int v = double(y - triTop) / triHeight * 255;
        int s = double(x - limL) / (2 * triWidth) * 255;
        hsv[1] = s;
        hsv[2] = v;

        wxCommandEvent *e = new wxCommandEvent(EVT_COLOR_PICKER_CHANGE, GetId());
        e->SetEventObject(this);
        wxQueueEvent(GetEventHandler(), e);

        Refresh();
        Update();
        if (event.LeftUp())
        {
            // finish drag
            this->ReleaseMouse();
            draging_triangle = false;
        }
    }
}
void ColorCircle::paintImage(wxImage &image)
{
    wxSize size = GetSize();
    int w = size.GetWidth();
    int h = size.GetHeight();
    int cw = w / 2;
    int ch = h / 2;
    unsigned char *imgData = image.GetData();
    int i = 0;
    wxColor bg = GetBackgroundColour();

    // loop and paint color
    for (int y = 0; y < h; y += 1)
    {
        for (int x = 0; x < w; x += 1)
        {
            int area = this->checkArea(x, y);
            if (area == 0) // background
            {
                imgData[i] = bg.Red();
                imgData[i + 1] = bg.Green();
                imgData[i + 2] = bg.Blue();
            }
            else if (area == 1) // circle
            {
                if (need_redraw_circle)
                {
                    //draw the circle
                    int cx = x - cw;
                    int cy = y - ch;
                    int angle = atan2(cy, cx) * 180 / 3.1415 + 180;
                    int rgb[3];
                    int hsv[3] = {angle, 255, 255};
                    Color::HSV2RGB(hsv, rgb);
                    imgData[i] = rgb[0];
                    imgData[i + 1] = rgb[1];
                    imgData[i + 2] = rgb[2];
                }
            }
            else
            {
                if (need_redraw_triangle) // triangle
                {
                    //draw triangle
                    int r = min(w, h) / 2 - 2;
                    int bandWidth = r * 0.2;
                    int triTop = ch - (r - bandWidth);
                    int triBottom = ch + sin(3.1415 / 6) * (r - bandWidth);
                    int triHeight = triBottom - triTop;
                    int triWidth = sin(3.1415 / 3) * (r - bandWidth) * (double(y - triTop) / triHeight);
                    int limL = cw - triWidth;
                    int limR = cw + triWidth;
                    int v = double(y - triTop) / triHeight * 255;
                    int s = double(x - limL) / (2 * triWidth) * 255;
                    int rgb[3];
                    int hsv[3] = {this->hsv[0], s, v};
                    Color::HSV2RGB(hsv, rgb);
                    imgData[i] = rgb[0];
                    imgData[i + 1] = rgb[1];
                    imgData[i + 2] = rgb[2];
                }
            }
            i += 3;
        }
    }
}
int ColorCircle::checkArea(int x, int y)
{
    //return 0 if not in area, 1 if in circle, 2 if in triangle
    wxSize size = GetSize();
    int w = size.GetWidth();
    int h = size.GetHeight();
    int r = min(w, h) / 2 - 2;
    int bandWidth = r * 0.2;
    w /= 2;
    h /= 2;
    double d = sqrt(pow(x - w, 2) + pow(y - h, 2));
    if (d < r && d > r - bandWidth)
    {
        return 1;
    }
    else
    {
        int triTop = h - (r - bandWidth);
        int triBottom = h + sin(3.1415 / 6) * (r - bandWidth);
        int triHeight = triBottom - triTop;
        int triWidth = sin(3.1415 / 3) * (r - bandWidth) * (double(y - triTop) / triHeight);
        int limL = w - triWidth;
        int limR = w + triWidth;
        if (y >= triTop && y <= triBottom && x >= limL && x <= limR)
        {
            return 2;
        }
    }
    return 0;
}
