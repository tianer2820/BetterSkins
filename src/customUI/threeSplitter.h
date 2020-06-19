#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/event.h>

#include <list>


#ifndef THREE_SPLITTER_H
#define THREE_SPLITTER_H

class ThreeSpliterWindow : public wxPanel
{
public:
    ThreeSpliterWindow(wxWindow *parent, const wxWindowID id, int minWinSize=10, int targetArea=6);

    void append(wxWindow *window);
    void setSplitter(int index, int pos);

protected:
    wxWindow *windowList[3];
    int splitter_list[2] = {10, 10};
    int win_min;
    int target_r;
    bool is_dragging = false;
    int drag_index = -1;

    void moveSplitter(int index, int pos);
    void checkSplitters();
    int collideTest(int x, int y);
    void onMouse(wxMouseEvent &event);
    void onPaint(wxPaintEvent &event);
    void onSize(wxSizeEvent &event);
    void updateWindows();
    virtual wxSize DoGetBestSize() const override;
};

#endif