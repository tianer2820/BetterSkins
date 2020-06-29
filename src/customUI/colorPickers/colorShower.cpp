#include "colorShower.hpp"

ColorShower::ColorShower(wxWindow *parent, wxWindowID id) : wxWindow(parent, id)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &ColorShower::onPaint, this);
    Bind(wxEVT_SIZE, &ColorShower::onSize, this);
    SetMinSize(wxSize(20, 20));
}

void ColorShower::setColor(int *rgb, int alpha)
{
    for (int i = 0; i < 3; i++)
    {
        rgba[i] = rgb[i];
    }
    rgba[3] = alpha;
    need_redraw_final_render = true;
    Refresh();
}

void ColorShower::onSize(wxSizeEvent &event)
{
    bg_checker = wxImage(event.GetSize());
    bg_checker.InitAlpha();
    need_redraw_bg = true;
    final_render = wxImage(event.GetSize());
    final_render.InitAlpha();
    need_redraw_final_render = true;
    Refresh();
    event.Skip();
}

void ColorShower::onPaint(wxPaintEvent &event)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    if (need_redraw_bg)
    {
        drawAlpha(bg_checker);
        need_redraw_bg = false;
    }
    if (need_redraw_final_render)
    {
        need_redraw_final_render = false;
        u_char *bg_data = bg_checker.GetData();
        u_char *bg_alpha = bg_checker.GetAlpha();
        u_char *render_data = final_render.GetData();
        u_char *render_alpha = final_render.GetAlpha();
        int w = bg_checker.GetWidth();
        int h = bg_checker.GetHeight();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                int index = y * w + x;
                if (x < w / 2)
                { // pure alpha checker
                    for (int i = 0; i < 3; i++)
                    {
                        render_data[index * 3 + i] = bg_data[index * 3 + 1];
                    }
                    render_alpha[index] = bg_alpha[index];
                }
                else
                { // alpha over
                    int rgba0[4];
                    for (int i = 0; i < 3; i++)
                    {
                        rgba0[i] = bg_data[index * 3 + i];
                    }
                    rgba0[3] = bg_alpha[index];

                    int out[4];
                    Color::alphaOver(rgba0, rgba, out);

                    for (int i = 0; i < 3; i++)
                    {
                        render_data[index * 3 + i] = out[i];
                    }
                    render_alpha[index] = out[3];
                }
            }
        }
    }
    dc.DrawBitmap(wxBitmap(final_render), 0, 0);
}
