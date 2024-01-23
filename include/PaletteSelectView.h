#ifndef _PALETTE_SELECT_VIEW_H_
#define _PALETTE_SELECT_VIEW_H_

#include "Direct2DEx.h"
#include "PaletteModel.h"
#include <vector>
#include <map>

class PaletteSelectView
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
	PaletteSelectView(Direct2DEx *const ap_direct2d, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList, const CM &a_mode);
	virtual ~PaletteSelectView();

	void Init(const SIZE &a_viewSize);
	void Paint(const PALETTE::MD &a_modelData);

	void AddColor(const DColor &a_color);

	DColor GetColor(const size_t &a_index);
	std::vector<DColor> GetColorList();
	const std::map<size_t, DRect> GetColorDataTable();
	const std::pair<size_t, DRect> &GetAddButtonData();

protected:
	const DRect GetColorRect(const size_t a_index);
};

#endif //!_PALETTE_SELECT_VIEW_H_