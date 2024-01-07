#ifndef _BUTTON_SHAPE_H_
#define _BUTTON_SHAPE_H_

#include "Utility.h"
#include <map>
#include <vector>

class ButtonShape
{
private:
	struct HUE_DATA
	{
		DPoint point;
		DColor color;
	};

private:
	Direct2DEx *const  mp_direct2d;
	const std::map<BST, DRect> &m_buttonTable;
	std::map<BST, void (ButtonShape::*)(const BSD &)> m_drawTable;
	IDWriteTextFormat *mp_textFormat;

	DColor m_textColor;
	DColor m_highlightColor;
	const float m_defaultTransparency;

	ID2D1PathGeometry *mp_curveGeometry;
	std::vector<DRect> m_strokShapeRects;
	ID2D1LinearGradientBrush *mp_gradientBrush;
	std::vector<ID2D1PathGeometry *> m_gradientGeometries;
	std::vector<HUE_DATA> m_hueDataList;
	DColor m_selectedBrushColor;
	ID2D1LinearGradientBrush *mp_colorShapeBrush;
	const float m_colorShapeMargin = 12.0f;
	std::vector<DRect> m_fadeShapeRects;
	const float m_fadeShapeMargin = 10.0f;

public:
	ButtonShape(Direct2DEx * const ap_direct2d, const std::map<BST, DRect> &a_buttonTable, const WindowDialog::THEME_MODE &a_mode);
	virtual ~ButtonShape();

	void SetColorMode(const WindowDialog::THEME_MODE &a_mode);
	void DrawButton(const BST &a_type, const BSD &a_data);
	
private:
	void InitCurveShapeData();
	void InitStrokyShapeData();
	void InitGradiationShapeData();
	void InitColorShapeData();
	void UpdateColorSymbolBrush();
	void InitFadeShapeData();

	void UpdateTextColorOnHover(const BST &a_type, const BSD &a_data);

	void DrawCurveShape(const BSD &a_data);
	void DrawRectangleShape(const BSD &a_data);
	void DrawCircleShape(const BSD &a_data);
	void DrawTextShape(const BSD &a_data);
	void DrawStrokeShape(const BSD &a_data);
	void DrawGradiationShape(const BSD &a_data);
	void DrawColorShape(const BSD &a_data);
	void DrawFadeShape(const BSD &a_data);
};

#endif //_BUTTON_SHAPE_H_