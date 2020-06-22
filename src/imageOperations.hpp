#if !defined(IMAGE_OPERATIONS_H)
#define IMAGE_OPERATIONS_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

void clearAlpha(wxImage& img);
void drawAlpha(wxImage& img);


void drawAlpha(wxImage &img)
{
    int w = img.GetWidth();
    int h = img.GetHeight();
    u_char *data = img.GetData();
    u_char *alpha = img.GetAlpha();
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int i = y * w + x;
            alpha[i] = 255;
            if ((x / 10 + y / 10) % 2 == 0)
            {
                // black block
                for (int c = 0; c < 3; c++)
                {
                    data[i * 3 + c] = 170;
                }
            }
            else
            {
                // white block
                for (int c = 0; c < 3; c++)
                {
                    data[i * 3 + c] = 220;
                }
            }
        }
    }
}

void clearAlpha(wxImage &img)
{
    int w = img.GetWidth();
    int h = img.GetHeight();
    u_char *alpha = img.GetAlpha();
    int data_length = w * h;
    for (int i = 0; i < data_length; i++)
    {
        alpha[i] = 0;
    }
}


#endif // IMAGE_OPERATIONS_H
