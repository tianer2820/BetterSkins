#if !defined(IMAGE_OPERATIONS_H)
#define IMAGE_OPERATIONS_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "color/color.h"

void clearAlpha(wxImage &img);
void drawAlpha(wxImage &img);

/**
 * this will draw the checker pattern on the whole image
 */
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

/**
 * set all pixels to transparent
 */
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

/**
 * lay the "upper" onto the "lower", using alpha blend.
 * The lower is modified.
 * If two image don't have the same size, size of "lower" will be used.
 */
void alphaOver(wxImage &lower, wxImage &upper)
{
    // alpha over the image
    int width = lower.GetWidth();
    int height = lower.GetHeight();
    u_char *lower_data = lower.GetData();
    u_char *lower_alpha = lower.GetAlpha();
    u_char *upper_data = upper.GetData();
    u_char *upper_alpha = upper.GetAlpha();
    for (int i = 0; i < width * height; i++)
    {
        int rgba0[] = {lower_data[i * 3], lower_data[i * 3 + 1], lower_data[i * 3 + 2], lower_alpha[i]};
        int rgba1[] = {upper_data[i * 3], upper_data[i * 3 + 1], upper_data[i * 3 + 2], upper_alpha[i]};
        int out[4];

        Color::alphaOver(rgba0, rgba1, out);
        for (int c = 0; c < 3; c++)
        {
            lower_data[i * 3 + c] = out[c];
        }
        lower_alpha[i] = out[3];
    }
}

#endif // IMAGE_OPERATIONS_H
