#if !defined(IMAGE_OPERATIONS_H)
#define IMAGE_OPERATIONS_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "color/color.h"



/**
 * this will draw the checker pattern on the whole image
 */
extern void drawAlpha(wxImage &img);

/**
 * set all pixels to transparent
 */
extern void clearAlpha(wxImage &img);

/**
 * lay the "upper" onto the "lower", using alpha blend.
 * The lower is modified.
 * If two image don't have the same size, size of "lower" will be used.
 */
void alphaOver(wxImage &lower, wxImage &upper);

#endif // IMAGE_OPERATIONS_H
