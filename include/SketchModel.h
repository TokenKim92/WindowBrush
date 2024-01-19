#ifndef _SKETCH_MODEL_H_
#define _SKETCH_MODEL_H_
#include <vector>
#include "Direct2D.h"
#include "WindowBrushModel.h"

namespace SKETCH {
	const size_t INVALID_INDEX = static_cast<size_t>(-1);

	const unsigned int WM_UPDATEMD = 24001;
	
	const size_t GRADIENT_BRUSH_COUNT = 10;

	typedef struct DEFAULT_DATA
	{
		unsigned int strokeWidth;
		unsigned int fontSize;
		float transparency;
		DColor color;
		size_t gradientBrushIndex;
	}DD;

	typedef struct MODEL_DATA
	{
		WINDOW_BRUSH::DT drawType;
		std::vector<DPoint> points;
		DRect rect;
		DD defaultData;
	}MD;
}

#endif //!_SKETCH_MODEL_H_