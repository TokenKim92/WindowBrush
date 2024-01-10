#ifndef _BUTTON_SHAPE_H_
#define _BUTTON_SHAPE_H_

#include "Direct2DEx.h"
#include "WindowBrushModel.h"
#include <map>
#include <vector>

class WindowBrushView : public Direct2DEx
{
private:
	std::map<WBBT, DRect> m_buttonTable;
	std::vector<DRect> m_dividerList;
	std::map<WBBT, void (WindowBrushView:: *)(const WBMD &)> m_drawTable;

	IDWriteTextFormat *mp_textFormat;
	DColor m_textColor;
	DColor m_highlightColor;
	const float m_defaultTransparency;

	// for curve button
	ID2D1PathGeometry *mp_curveGeometry;
	// for stroke button
	std::vector<DRect> m_strokShapeRects;
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
	void Paint(const WBMD &a_modelData);

	const std::map<WBBT, DRect> GetButtonTable();
	void SetColorMode(const CM &a_mode);
	void UpdateColorSymbolBrush(const DColor &a_color);

private:
	void InitButtonRects();
	void InitDivider();

	void InitCurveShapeData();
	void InitStrokyShapeData();
	void InitGradiationShapeData();
	void InitColorShapeData();
	void InitFadeShapeData();

	void UpdateTextColorOnHover(const WBBT &a_type, const WBMD &a_data);

	void DrawCurveShape(const WBMD &a_data);
	void DrawRectangleShape(const WBMD &a_data);
	void DrawCircleShape(const WBMD &a_data);
	void DrawTextShape(const WBMD &a_data);
	void DrawStrokeShape(const WBMD &a_data);
	void DrawGradiationShape(const WBMD &a_data);
	void DrawColorShape(const WBMD &a_data);
	void DrawFadeShape(const WBMD &a_data);
};

#endif //_BUTTON_SHAPE_H_