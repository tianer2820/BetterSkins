#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/event.h>

#include <list>
#include "threeSplitter.h"


    ThreeSpliterWindow::ThreeSpliterWindow(wxWindow *parent, const wxWindowID id, int minWinSize, int targetArea) : wxPanel(parent, id)
    {
        win_min = minWinSize;
        target_r = targetArea;
        for (int i = 0; i < 3; i += 1)
        {
            windowList[i] = NULL;
        }
        this->SetBackgroundStyle(wxBG_STYLE_PAINT);

        this->Bind(wxEVT_MOTION, &ThreeSpliterWindow::onMouse, this);
        this->Bind(wxEVT_LEFT_DOWN, &ThreeSpliterWindow::onMouse, this);
        this->Bind(wxEVT_LEFT_UP, &ThreeSpliterWindow::onMouse, this);
        this->Bind(wxEVT_ENTER_WINDOW, &ThreeSpliterWindow::onMouse, this);
        this->Bind(wxEVT_LEAVE_WINDOW, &ThreeSpliterWindow::onMouse, this);

        this->Bind(wxEVT_SIZE, &ThreeSpliterWindow::onSize, this);
        this->Bind(wxEVT_PAINT, &ThreeSpliterWindow::onPaint, this);
    }
    void ThreeSpliterWindow::append(wxWindow *window)
    {
        for (int i = 0; i < 3; i += 1)
        {
            if (this->windowList[i] == NULL)
            {
                this->windowList[i] = window;
                return;
            }
        }
        throw "too much windows!";
    }
    void ThreeSpliterWindow::setSplitter(int index, int pos)
    {
        this->splitter_list[index] = pos;
        this->Refresh();
    }

    void ThreeSpliterWindow::moveSplitter(int index, int pos)
    {
        int width = this->GetSize().GetWidth();
        if (index == 0)
        {
            splitter_list[0] = pos;
        }
        else if (index == 1)
        {
            splitter_list[1] = width - pos;
        }
        else
        {
            throw "unknown index";
        }
        this->checkSplitters();
    }
    void ThreeSpliterWindow::checkSplitters()
    {
        int width = this->GetSize().GetWidth();
        bool locks[] = {false, false, false};

        if (splitter_list[0] < win_min + target_r)
        {
            splitter_list[0] = win_min + target_r;

            locks[0] = true;
        }

        if (splitter_list[1] < win_min + target_r)
        {
            splitter_list[1] = win_min + target_r;
            locks[2] = true;
        }
        if (splitter_list[0] + win_min + target_r * 2 > width - splitter_list[1])
        {
            locks[1] = true;
            if (!locks[2])
            {
                splitter_list[1] = width - (splitter_list[0] + win_min + target_r * 2);
            }
            else
            {
                this->moveSplitter(0, width - splitter_list[1] - win_min - target_r * 2);
            }
        }
    }
    int ThreeSpliterWindow::collideTest(int x, int y)
    {
        int width = this->GetSize().GetWidth();
        if (splitter_list[0] - target_r <= x && x <= splitter_list[0] + target_r)
        {
            return 0;
        }
        else if (width - splitter_list[1] - target_r <= x && x <= width - splitter_list[1] + target_r)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    void ThreeSpliterWindow::onMouse(wxMouseEvent &event)
    {
        int index = this->collideTest(event.GetX(), event.GetY());

        if (index != -1 && !this->is_dragging)
        {
            this->SetCursor(wxCursor(wxCURSOR_SIZEWE));
        }
        if (event.Leaving() && !this->is_dragging)
        {
            this->SetCursor(wxCursor(wxCURSOR_DEFAULT));
        }
        if (event.LeftDown())
        {
            if (index != -1)
            {
                this->CaptureMouse();
                this->is_dragging = true;
                this->drag_index = index;
            }
        }

        if (this->is_dragging)
        {
            this->moveSplitter(drag_index, event.GetX());
            if (event.LeftUp())
            {
                this->SetCursor(wxCursor(wxCURSOR_DEFAULT));
                this->ReleaseMouse();
                this->is_dragging = false;
                this->drag_index = -1;
            }
            this->Refresh();
        }
        event.Skip();
    }
    void ThreeSpliterWindow::onPaint(wxPaintEvent &event)
    {
        updateWindows();
        Refresh();
        wxAutoBufferedPaintDC dc(this);
        dc.SetPen(wxPen(wxColor(0, 0, 0), 1, wxPENSTYLE_SOLID));
        dc.Clear();
        int height = this->GetSize().GetHeight();
        int width = this->GetSize().GetWidth();
        dc.DrawLine(width - splitter_list[1], 0, width - splitter_list[1], height);
        dc.DrawLine(splitter_list[0], 0, splitter_list[0], height);
    }
    void ThreeSpliterWindow::onSize(wxSizeEvent &event)
    {
        this->updateWindows();
        event.Skip();
    }
    void ThreeSpliterWindow::updateWindows()
    {
        int wid = this->GetSize().GetWidth();
        int hei = this->GetSize().GetHeight();

        checkSplitters();
        windowList[0]->SetPosition(wxPoint(0, 0));
        windowList[0]->SetSize(wxSize(splitter_list[0] - target_r, hei));

        windowList[1]->SetPosition(wxPoint(splitter_list[0] + target_r, 0));
        windowList[1]->SetSize(wxSize(wid - splitter_list[0] - splitter_list[1] - target_r * 2, hei));

        windowList[2]->SetPosition(wxPoint(wid - splitter_list[1] + target_r, 0));
        windowList[2]->SetSize(wxSize(splitter_list[1] - target_r, hei));
    }
    wxSize ThreeSpliterWindow::DoGetBestSize() const
    {
        int width = 0;
        int height = 0;
        for (int i = 0; i < 3; i += 1)
        {
            wxWindow *win = windowList[i];
            wxSize s = win->GetBestSize();
            width += s.GetWidth();
            height = std::max(height, s.GetHeight());
        }
        width += target_r * 4;
        width = std::max(width, win_min * 3);
        height = std::max(height, win_min * 3);
        return wxSize(width, height);
    }
