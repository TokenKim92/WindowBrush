#ifndef _WINDOW_BRUSH_MODEL_H_
#define _WINDOW_BRUSH_MODEL_H_

#include <d2d1.h>

const double CONVERT_RADIAN = 0.0174532888;

typedef enum class WINDOW_BRUSH_BUTTON_TYPE
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
} WBBT;

typedef struct WINDOW_BRUSH_MODEL_DATA
{
	WBBT hoverArea;
	WBBT drawMode;
	bool isGradientMode;
	bool isFadeMode;
	D2D1_COLOR_F selectedColor;
} WBMD;

#endif // !_WINDOW_BRUSH_MODEL_H_