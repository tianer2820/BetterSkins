#if !defined(COLOR_H)
#define COLOR_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <string>

using namespace std;

class Color
{
public:
    virtual void setRGB(int *rgb) = 0;
    virtual void setHSV(int *hsv) = 0;
    virtual void setHEX(string s) = 0;
    virtual void setGray(int gray) = 0;

    virtual void getRGB(int *out) const = 0;
    virtual void getHSV(int *out) const = 0;
    virtual string getHEX() const = 0;
    virtual int getGray() const = 0;

    void setAlpha(u_char alpha);
    u_char getAlpha() const;

    virtual string toString() const = 0;
    virtual wxColor toWxColor() const;
    virtual Color* copy() const = 0;

    static void RGB2HSV(int *rgb, int *out);
    static void HSV2RGB(int *hsv, int *out);
    static string RGB2HEX(int *rgb);
    static void HEX2RGB(string hex, int *out);
    static int RGB2GRAY(int* rgb);

    static void alphaOver(int *rgba0, int *rgba1, int *out);
    
protected:
    u_char a;
};

class RGBColor : public Color
{
public:
    RGBColor(u_char r = 0, u_char g = 0, u_char b = 0, u_char a = 255);
    RGBColor(int *rgb, u_char a = 255);
    virtual void setRGB(int *rgb);
    virtual void setHSV(int *hsv);
    virtual void setHEX(string s);
    virtual void setGray(int gray);

    virtual void getRGB(int *out) const;
    virtual void getHSV(int *out) const;
    virtual string getHEX() const;
    virtual int getGray() const;

    virtual string toString() const;
    virtual Color* copy() const;

protected:
    u_char rgb[3];
};

class HSVColor : public Color
{
public:
    HSVColor(u_char h = 0, u_char s = 0, u_char v = 0, u_char a = 255);
    HSVColor(int *hsv, u_char a = 255);
    virtual void setRGB(int *rgb);
    virtual void setHSV(int *hsv);
    virtual void setHEX(string s);
    virtual void setGray(int gray);

    virtual void getRGB(int *out) const;
    virtual void getHSV(int *out) const;
    virtual string getHEX() const;
    virtual int getGray() const;

    virtual string toString() const;
    virtual Color* copy() const;

protected:
    u_short hsv[3];
};

#endif // COLOR_H