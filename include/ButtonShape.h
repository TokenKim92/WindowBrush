#ifndef _BUTTON_SHAPE_H_
#define _BUTTON_SHAPE_H_

#include <map>
#include <vector>

class ButtonShape
{
public:
	enum TYPE
	{
		NONE = -1,
		CURVE = 0,
		RECTANGLE,
		CIRCLE,
		TEXT,
		STROKE,
		GRADIATION,
		COLOR,
		FADE
	};

	struct BUTTON_SHAPE_DATA
	{
		ButtonShape::TYPE hoverArea;
	};

private:
	struct HUE_DATA
	{
		DPoint point;
		DColor color;
	};

private:
	Direct2DEx *const  mp_direct2d;
	const std::map<TYPE, DRect> &m_buttonTable;
	std::map<TYPE, void (ButtonShape::*)(const BUTTON_SHAPE_DATA &)> m_drawTable;
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
	ButtonShape(Direct2DEx * const ap_direct2d, const std::map<TYPE, DRect> &a_buttonTable, const WindowDialog::THEME_MODE &a_mode);
	virtual ~ButtonShape();

	void SetColorMode(const WindowDialog::THEME_MODE &a_mode);
	void DrawButton(const TYPE &a_type, const BUTTON_SHAPE_DATA &a_data);
	
private:
	void InitCurveShapeData();
	void InitStrokyShapeData();
	void InitGradiationShapeData();
	void InitColorShapeData();
	void UpdateColorSymbolBrush();
	void InitFadeShapeData();

	void UpdateTextColorOnHover(const bool isHover);

	void DrawCurveShape(const BUTTON_SHAPE_DATA &a_data);
	void DrawRectangleShape(const BUTTON_SHAPE_DATA &a_data);
	void DrawCircleShape(const BUTTON_SHAPE_DATA &a_data);
	void DrawTextShape(const BUTTON_SHAPE_DATA &a_data);
	void DrawStrokeShape(const BUTTON_SHAPE_DATA &a_data);
	void DrawGradiationShape(const BUTTON_SHAPE_DATA &a_data);
	void DrawColorShape(const BUTTON_SHAPE_DATA &a_data);
	void DrawFadeShape(const BUTTON_SHAPE_DATA &a_data);
};

#endif //_BUTTON_SHAPE_H_