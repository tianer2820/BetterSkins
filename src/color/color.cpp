#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <string>
#include <sstream>
#include <iomanip>
#include <math.h>
#include "color.h"

// Color

void Color::setAlpha(u_char alpha)
{
    this->a = alpha;
}
u_char Color::getAlpha()
{
    return this->a;
}
void Color::RGB2HSV(int *rgb, int *out)
{
    double rp = double(rgb[0]) / 255;
    double gp = double(rgb[1]) / 255;
    double bp = double(rgb[2]) / 255;

    double cmax = std::max(rp, std::max(gp, bp));
    double cmin = std::min(rp, std::min(gp, bp));
    double delta = cmax - cmin;

    double h, s, v;

    if (delta == 0)
    {
        h = 0;
    }
    else if (cmax == rp)
    {
        h = (((int)(60 * (gp - bp) / delta) + 360) % 360);
    }
    else if (cmax == gp)
    {
        h = 60 * (((bp - rp) / delta) + 2);
    }
    else
    {
        h = 60 * (((rp - gp) / delta) + 4);
    }
    if (cmax == 0)
    {
        s = 0;
    }
    else
    {
        s = delta / cmax;
    }
    v = cmax;
    out[0] = int(h);
    out[1] = int(s * 255);
    out[2] = int(v * 255);
}
void Color::HSV2RGB(int *hsv, int *out)
{
    int h = hsv[0];
    int s = hsv[1];
    int v = hsv[2];

    h %= 360;
    double s1 = double(s) / 255;
    double v1 = double(v) / 255;
    double c = v1 * s1;
    double x = c * (1 - double(abs((h) % 120 - 60)) / 60);
    double m = v1 - c;

    double rp, gp, bp;
    if (0 <= h && h < 60)
    {
        rp = c;
        gp = x;
        bp = 0;
    }
    else if (60 <= h && h < 120)
    {
        rp = x;
        gp = c;
        bp = 0;
    }
    else if (120 <= h && h < 180)
    {
        rp = 0;
        gp = c;
        bp = x;
    }
    else if (180 <= h && h < 240)
    {
        rp = 0;
        gp = x;
        bp = c;
    }
    else if (240 <= h && h < 300)
    {
        rp = x;
        gp = 0;
        bp = c;
    }
    else
    {
        rp = c;
        gp = 0;
        bp = x;
    }

    out[0] = 255 * (rp + m);
    out[1] = 255 * (gp + m);
    out[2] = 255 * (bp + m);
}
string Color::RGB2HEX(int *rgb)
{
    ostringstream out;
    for (int i = 0; i < 3; i += 1)
    {
        out << std::hex << std::setfill('0') << std::setw(2);
        out << (static_cast<short>(rgb[i]) & 0xff);
    }
    return out.str();
}
void Color::HEX2RGB(string hex, int *out)
{
    assert(hex.length() == 6);
    try
    {
        int rgb = std::stoi(hex, nullptr, 16);
        int r = rgb >> 16 & 255;
        int g = rgb >> 8 & 255;
        int b = rgb & 255;
        out[0] = r;
        out[1] = g;
        out[2] = b;
    }
    catch (invalid_argument)
    {
        for (int i = 0; i < 3; i += 1)
        {
            out[i] = 0;
        }
    }
}
int Color::RGB2GRAY(int *rgb)
{
    int gray = (rgb[0] * 299 + rgb[1] * 587 + rgb[2] * 114 + 500) / 1000;
    return gray;
}
void Color::alphaOver(int *rgba0, int *rgba1, int *out)
{
    double a1 = static_cast<double>(rgba0[3]) / 255;
    double a2 = static_cast<double>(rgba1[3]) / 255;
    int ap = int((a1 + a2 - a1 * a2) * 255);
    int rp = int((rgba0[0] * a1 * (1 - a2) + rgba1[0] * a2) / (a1 + a2 - a1 * a2));
    int gp = int((rgba0[1] * a1 * (1 - a2) + rgba1[1] * a2) / (a1 + a2 - a1 * a2));
    int bp = int((rgba0[2] * a1 * (1 - a2) + rgba1[2] * a2) / (a1 + a2 - a1 * a2));
    out[0] = rp;
    out[1] = gp;
    out[2] = bp;
    out[3] = ap;
}
void Color::clearAlpha(wxImage &img)
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
wxColor Color::toWxColor()
{
    int rgb[3];
    getRGB(rgb);
    return wxColor(rgb[0], rgb[1], rgb[2], a);
}
void Color::drawAlpha(wxImage &img)
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

// RGBColor
RGBColor::RGBColor(u_char r, u_char g, u_char b, u_char a)
{
    this->rgb[0] = r;
    this->rgb[1] = g;
    this->rgb[2] = b;
    setAlpha(a);
}
RGBColor::RGBColor(int *rgb, u_char a)
{
    for (int i = 0; i < 3; i += 1)
    {
        this->rgb[i] = u_char(rgb[i]);
    }
    setAlpha(a);
}

void RGBColor::setRGB(int *rgb)
{
    for (int i = 0; i < 3; i += 1)
    {
        this->rgb[i] = u_char(rgb[i]);
    }
}
void RGBColor::setHSV(int *hsv)
{
    int out[3];
    Color::HSV2RGB(hsv, out);
    setRGB(out);
}
void RGBColor::setHEX(string s)
{
    int out[3];
    Color::HEX2RGB(s, out);
    setRGB(out);
}
void RGBColor::setGray(int gray)
{
    for (int i = 0; i < 3; i++)
    {
        this->rgb[i] = gray;
    }
}
void RGBColor::getRGB(int *out)
{
    for (int i = 0; i < 3; i += 1)
    {
        out[i] = this->rgb[i];
    }
}
void RGBColor::getHSV(int *out)
{
    int rgbi[3];
    getRGB(rgbi);
    Color::RGB2HSV(rgbi, out);
}
string RGBColor::getHEX()
{
    int rgbi[3];
    getRGB(rgbi);
    return Color::RGB2HEX(rgbi);
}
int RGBColor::getGray()
{
    int irgb[3];
    getRGB(irgb);
    return Color::RGB2GRAY(irgb);
}
string RGBColor::toString()
{
    stringstream out;
    out.clear();
    out << "<RGBColorObject: R=" << (int)rgb[0] << " G=" << (int)rgb[1] << " B=" << (int)rgb[2] << ">";
    return out.str();
}
Color *RGBColor::copy()
{
    return new RGBColor(*this);
}

// HSV Color

HSVColor::HSVColor(u_char h, u_char s, u_char v, u_char a)
{
    hsv[0] = h;
    hsv[1] = s;
    hsv[2] = v;
    this->a = a;
}
HSVColor::HSVColor(int *hsv, u_char a)
{
    for (int i = 0; i < 3; i += 1)
    {
        this->hsv[i] = hsv[i];
    }
    this->a = a;
}

void HSVColor::setRGB(int *rgb)
{
    int out[3];
    Color::RGB2HSV(rgb, out);
    this->setHSV(out);
}
void HSVColor::setHSV(int *hsv)
{
    for (int i = 0; i < 3; i += 1)
    {
        this->hsv[i] = hsv[i];
    }
}
void HSVColor::setHEX(string s)
{
    int out[3];
    Color::HEX2RGB(s, out);
    this->setRGB(out);
}
void HSVColor::setGray(int gray)
{
    hsv[0] = 0;
    hsv[1] = 0;
    hsv[2] = gray;
}
void HSVColor::getRGB(int *out)
{
    int in[3];
    this->getHSV(in);
    Color::HSV2RGB(in, out);
}
void HSVColor::getHSV(int *out)
{
    for (int i = 0; i < 3; i += 1)
    {
        out[i] = this->hsv[i];
    }
}
string HSVColor::getHEX()
{
    int rgb[3];
    this->getRGB(rgb);
    return Color::RGB2HEX(rgb);
}
int HSVColor::getGray()
{
    return hsv[2];
}
string HSVColor::toString()
{
    stringstream out;
    out.clear();
    out << "<HSVColorObject: H=" << (int)hsv[0] << " S=" << (int)hsv[1] << " V=" << (int)hsv[2] << ">";
    return out.str();
}
Color *HSVColor::copy()
{
    return new HSVColor(*this);
}
