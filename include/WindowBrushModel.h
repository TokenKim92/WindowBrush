#ifndef _WINDOW_BRUSH_MODEL_H_
#define _WINDOW_BRUSH_MODEL_H_

#include <d2d1.h>

namespace WINDOW_BRUSH {

	const double CONVERT_RADIAN = 0.0174532888;
	const float DEFAULT_TRANSPARENCY = 0.6f;

	typedef enum class BUTTON_TYPE
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
	} BT;

	typedef struct MODEL_DATA
	{
		BT hoverArea;
		BT drawMode;
		unsigned int strokeWidth;
		unsigned int fontSize;
		bool isGradientMode;
		D2D1_COLOR_F selectedColor;
		bool isFadeMode;

		RECT selectedScreenRect;
		unsigned int fadeTimer; //ms
		float colorOpacity;
	} MD;

}
#endif // !_WINDOW_BRUSH_MODEL_H_