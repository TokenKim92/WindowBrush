#ifndef _COLOR_SELECT_VIEW_H_
#define _COLOR_SELECT_VIEW_H_

#include "Direct2DEx.h"
#include "ColorModel.h"
#include <vector>
#include <map>

class ColorSelectView
{
protected:
	Direct2DEx *const mp_direct2d;
	ID2D1StrokeStyle *mp_addButtonStroke;

	const std::vector<DColor> &m_colorList;
	std::map<size_t, std::pair<DColor, DRect>> m_colorDataTable;	// key is a index
	std::pair<size_t, DColor> m_selectedColorData;					// first data of pair is a index
	std::pair<size_t, DRect> m_addButtonData;						// first data of pair is a index

	DColor m_mainColor;
	DColor m_oppositeColor;
	DPoint m_colorCircleStartPoint;

	size_t m_colorCountPerWidth;
	size_t m_colorCountPerHeight;
	size_t m_maxColorDataSize;

public:
	ColorSelectView(Direct2DEx *const ap_direct2d, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList, const CM &a_mode);
	virtual ~ColorSelectView();

	void Init(const SIZE &a_viswSize);
	void Paint(const CMD &a_modelData);

	DColor GetColor(const size_t &a_index);
	const std::map<size_t, DRect> GetColorDataTable();
	const std::pair<size_t, DRect> &GetAddButtonData();

protected:
	const DRect GetColorRect(const size_t a_index);
};

#endif //!_COLOR_SELECT_VIEW_H_