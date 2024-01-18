#ifndef _BUTTON_SHAPE_H_
#define _BUTTON_SHAPE_H_

#include "Direct2DEx.h"
#include "WindowBrushModel.h"
#include <map>
#include <vector>

class WindowBrushView : public Direct2DEx
{
private:
	std::map<WINDOW_BRUSH::BT, DRect> m_buttonTable;
	std::vector<DRect> m_dividerList;
	std::map<WINDOW_BRUSH::BT, void (WindowBrushView:: *)(const WINDOW_BRUSH::MD &)> m_drawTable;

	IDWriteTextFormat *mp_textFormat;
	DColor m_textColor;
	DColor m_highlightColor;

	// for curve button
	ID2D1PathGeometry *mp_curveGeometry;
	// for stroke button
	std::vector<DRect> m_strokeShapeRects;
	// for gradient button
	ID2D1LinearGradientBrush *mp_gradientBrush;
	std::vector<ID2D1PathGeometry *> m_gradientGeometries;
	// for color button
	std::vector<std::pair<DColor, DPoint>> m_hueDataList;
	ID2D1LinearGradientBrush *mp_colorShapeBrush;
	const float m_colorShapeMargin;
	// for fade button
	std::vector<DRect> m_fadeShapeRects;
	const float m_fadeShapeMargin;

public:
	WindowBrushView(const HWND &ah_window, const CM &a_mode, const RECT *const ap_viewRect = nullptr);
	virtual ~WindowBrushView();

	virtual int Create() override;
	void Paint(const WINDOW_BRUSH::MD &a_modelData);

	const std::map<WINDOW_BRUSH::BT, DRect> GetButtonTable();
	void SetColorMode(const CM &a_mode);
	void UpdateColorSymbolBrush(const DColor &a_color);

private:
	void UpdateTextColorOnHover(const WINDOW_BRUSH::BT &a_type, const WINDOW_BRUSH::MD &a_data);

	void DrawCurveShape(const WINDOW_BRUSH::MD &a_data);
	void DrawRectangleShape(const WINDOW_BRUSH::MD &a_data);
	void DrawCircleShape(const WINDOW_BRUSH::MD &a_data);
	void DrawTextShape(const WINDOW_BRUSH::MD &a_data);
	void DrawStrokeShape(const WINDOW_BRUSH::MD &a_data);
	void DrawGradientShape(const WINDOW_BRUSH::MD &a_data);
	void DrawColorShape(const WINDOW_BRUSH::MD &a_data);
	void DrawFadeShape(const WINDOW_BRUSH::MD &a_data);
};

#endif //_BUTTON_SHAPE_H_