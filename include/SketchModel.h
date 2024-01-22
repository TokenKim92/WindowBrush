#ifndef _SKETCH_MODEL_H_
#define _SKETCH_MODEL_H_

#include "Direct2D.h"
#include "WindowBrushModel.h"
#include <vector>
#include <string>

namespace SKETCH {
	const size_t INVALID_INDEX = static_cast<size_t>(-1);

	const unsigned int FPS = 30;
	const unsigned int FPS_TIME = 1000 / FPS;

	const unsigned int WM_UPDATE_MODEL_DATA = 24001;
	const unsigned int WM_SET_TEXTOUTLINE_MODE = WM_UPDATE_MODEL_DATA + 1;
	const unsigned int WM_ON_EDIT_MAX_LEGNTH = WM_UPDATE_MODEL_DATA + 2;
	
	const size_t GRADIENT_BRUSH_COUNT = 10;

	typedef struct DEFAULT_DATA
	{
		unsigned int strokeWidth;
		unsigned int fontSize;
		float opacity;
		DColor color;
		size_t gradientBrushIndex;
	}DD;

	typedef struct MODEL_DATA
	{
		WINDOW_BRUSH::DT drawType;
		std::vector<DPoint> points;
		DRect rect;
		std::wstring text;
		DD defaultData;
	}MD;
}

#endif //!_SKETCH_MODEL_H_