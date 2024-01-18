#ifndef _SKETCH_VIEW_H_
#define _SKETCH_VIEW_H_

#include "Direct2DEx.h"
#include "SketchModel.h"

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

	HBITMAP mh_screenBitmap;
	const RECT m_physicalRect;
	DRect m_viewRect;

	CS m_colorSet;

public:
	SketchView(const HWND ah_window, const HBITMAP &ah_screenBitmap, const RECT &a_physicalRect, const CM &a_mode);
	virtual ~SketchView();

	virtual int Create() override;
	void Paint(const SKETCH::MD &a_modelData);

protected:
	unsigned char GetAverageBrightness(const HBITMAP &ah_bitmap);
};

#endif //!_SKETCH_VIEW_H_