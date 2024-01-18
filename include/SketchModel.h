#ifndef _SKETCH_MODEL_H_
#define _SKETCH_MODEL_H_
#include <vector>
#include "Direct2D.h"

namespace SKETCH {
	const size_t INVALID_INDEX = static_cast<size_t>(-1);

	const unsigned int WM_UPDATEMD = 24001;
	
	const size_t GRADIENT_BRUSH_COUNT = 10;

	typedef struct CURVE_DATA
	{
		std::vector<DPoint> points;
		unsigned int strokeWidth;
		float transparency;
		DColor color;
		size_t gradientBrushIndex;
	}CD;

	typedef struct MODEL_DATA
	{
		std::vector<SKETCH::CD> curveDataList;
	}MD;
}

#endif //!_SKETCH_MODEL_H_