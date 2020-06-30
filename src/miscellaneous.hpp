#if !defined(MISCELLANEOUS_H)
#define MISCELLANEOUS_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

wxRect reCalcRect(const wxRect& rect){
    int x, y, w, h;
    if (rect.width < 0)
    {
        x = rect.x + rect.width;
        w = -rect.width;
    } else{
        x = rect.x;
        w = rect.width;
    }
    if(rect.height < 0){
        y = rect.y + rect.height;
        h = -rect.height;
    } else{
        y = rect.y;
        h = rect.height;
    }
    return wxRect(x, y, w, h);
}

#endif // MISCELLANEOUS_H
