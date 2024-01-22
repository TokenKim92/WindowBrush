#ifndef _WINDOW_BRUSH_MODEL_H_
#define _WINDOW_BRUSH_MODEL_H_

#include "WindowDialog.h"

namespace WINDOW_BRUSH {

	const unsigned int WM_KILLED_SKETCH = 23001;

	const double CONVERT_RADIAN = 0.0174532888;
	const float DEFAULT_TRANSPARENCY = 0.6f;

	const unsigned int DIALOG_WIDTH = 80;
	const unsigned int DIALOG_HEIGHT = 390;

	const float BUTTON_X_MARGIN = 10.0f;

	typedef enum class BUTTON_TYPE
	{
		NONE,
		CURVE,
		RECTANGLE,
		CIRCLE,
		TEXT,
		STROKE,
		GRADIENT,
		COLOR,
		FADE
	} BT;

	typedef enum class DRAW_TYPE
	{
		NONE,
		CURVE,
		RECTANGLE,
		CIRCLE,
		TEXT_OUTLINE,
		TEXT_TYPING,
		TEXT
	}DT;

	typedef struct MODEL_DATA
	{
		BT hoverButtonType;
		DT drawType;
		unsigned int strokeWidth;
		unsigned int fontSize;
		bool isGradientMode;
		D2D1_COLOR_F selectedColor;
		bool isFadeMode;

		RECT selectedScreenRect;
		unsigned int fadeTimer; //ms
		float colorOpacity;
	} MD;

	typedef struct INFO_DIALOG_DATA
	{
		WindowDialog *windowBrushDialog;
		WindowDialog *infoDialog;
	}IDD;

}
#endif // !_WINDOW_BRUSH_MODEL_H_