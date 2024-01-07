#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <d2d1.h>

typedef enum class BUTTON_SHAPE_TYPE
{
	NONE = -1,
	CURVE = 0,
	RECTANGLE,
	CIRCLE,
	TEXT,
	STROKE,
	GRADIATION,
	COLOR,
	FADE
} BST;

typedef struct BUTTON_SHAPE_DATA
{
	BST hoverArea;
} BSD;

D2D1_COLOR_F fromHueToColorF(const float hue);
bool PointInRectF(const D2D1_RECT_F &ap_rect, const POINT &ap_pos);

#endif //_UTILITY_H_