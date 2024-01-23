#ifndef _SKETCH_VIEW_H_
#define _SKETCH_VIEW_H_

#include "Direct2DEx.h"
#include "SketchModel.h"
#include <map>

class SketchView : public Direct2DEx
{
protected:
	typedef struct COLOR_SET
	{
		DColor highlight;
		DColor extentColor;
		DColor extentInvertColor;
	}CS;

protected:
	ID2D1Bitmap *mp_screenBitmap;
	ID2D1StrokeStyle *mp_dashStroke;
	std::map<size_t, ID2D1LinearGradientBrush *> m_gradientTable;

	HBITMAP mh_screenBitmap;
	const RECT m_physicalRect;
	DRect m_viewRect;

	CS m_colorSet;

public:
	SketchView(const HWND ah_window, const HBITMAP &ah_screenBitmap, const RECT &a_physicalRect, const CM &a_mode);
	virtual ~SketchView();

	virtual int Create() override;
	void Paint(const std::vector<SKETCH::MD> &a_modelDataList);

protected:
	// this method is not used for performance 
	unsigned char GetAverageBrightness(const HBITMAP &ah_bitmap);
};

#endif //!_SKETCH_VIEW_H_