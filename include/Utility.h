#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <d2d1.h>
#include <string>

const double PI = 3.14159265358979;

bool IsSameColor(const D2D1_COLOR_F &a_color1, const D2D1_COLOR_F &a_color2);
D2D1_COLOR_F FromHueToColor(const float &hue);
float GetBrightness(const D2D1_COLOR_F &a_color);
std::wstring FloatToHexWString(const float &a_value);

bool PointInRect(const D2D1_RECT_F &a_rect, const POINT &a_pos);
void ExpandRect(D2D1_RECT_F &a_rect, const float &a_offset);
void ShrinkRect(D2D1_RECT_F &a_rect, const float &a_offset);

#endif //_UTILITY_H_