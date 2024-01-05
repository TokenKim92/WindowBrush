#include "Direct2DEx.h"
#include "WindowDialog.h"
#include "ButtonShape.h"
#include "ColorPalette.h"
#include "Utility.h"
#include <direct.h>
#include <vector>

//#define SHOW_BUTTON_AREA
#define CONVERT_RADIAN		 0.0174532888;

extern ApplicationCore *gp_appCore;

ButtonShape::ButtonShape(Direct2DEx *const ap_direct2d, const std::map<TYPE, DRect> &a_buttonTable, const WindowDialog::THEME_MODE &a_mode) :
	mp_direct2d(ap_direct2d),
	m_buttonTable(a_buttonTable),
	m_defaultTransparency(0.7f)
{
	SetColorMode(a_mode);

	m_drawTable.insert({ TYPE::CURVE, &ButtonShape::DrawCurveShape });
	m_drawTable.insert({ TYPE::RECTANGLE, &ButtonShape::DrawRectangleShape });
	m_drawTable.insert({ TYPE::CIRCLE, &ButtonShape::DrawCircleShape });
	m_drawTable.insert({ TYPE::TEXT, &ButtonShape::DrawTextShape });
	m_drawTable.insert({ TYPE::STROKE, &ButtonShape::DrawStrokeShape });
	m_drawTable.insert({ TYPE::GRADIATION, &ButtonShape::DrawGradiationShape });
	m_drawTable.insert({ TYPE::COLOR, &ButtonShape::DrawColorShape });
	m_drawTable.insert({ TYPE::FADE, &ButtonShape::DrawFadeShape });

	mp_textFormat = ap_direct2d->CreateTextFormat(L"Times New Roman", 30.0f, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL);
	mp_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	InitCurveShapeData();
	InitStrokyShapeData();
	InitGradiationShapeData();
	InitColorShapeData();
	InitFadeShapeData();
}

ButtonShape::~ButtonShape()
{
	InterfaceRelease(&mp_textFormat);
	InterfaceRelease(&mp_curveGeometry);
}

void ButtonShape::SetColorMode(const WindowDialog::THEME_MODE &a_mode)
{
	if (WindowDialog::THEME_MODE::LIGHT_MODE == a_mode) {
		m_color = RGB_TO_COLORF(NEUTRAL_600);
		m_highlightColor = RGB_TO_COLORF(SKY_300);
	}
	else {
		m_color = RGB_TO_COLORF(NEUTRAL_400);
		m_highlightColor = RGB_TO_COLORF(VIOLET_600);
	}

	m_color.a = m_defaultTransparency;
	m_highlightColor.a = m_defaultTransparency;
}

void ButtonShape::DrawButton(const TYPE &a_type)
{
	auto drawFunction = m_drawTable.at(a_type);
	(this->*drawFunction)();
}

#include <d2d1helper.h>

void ButtonShape::DrawCurveShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::CURVE));
#endif 

	if (nullptr != mp_curveGeometry) {
		const auto rect = m_buttonTable.at(TYPE::CURVE);
		const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
		const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;

		mp_direct2d->SetMatrixTransform(D2D1::Matrix3x2F::Rotation(10, { centerPosX, centerPosY }));
		mp_direct2d->DrawGeometry(mp_curveGeometry);
		mp_direct2d->SetMatrixTransform(D2D1::Matrix3x2F::Rotation(0, { centerPosX, centerPosY }));
	}
}

void ButtonShape::DrawRectangleShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::RECTANGLE));
#endif 

	auto rect = m_buttonTable.at(TYPE::RECTANGLE);
	const float margin = 7.0f;
	rect.left += margin;
	rect.top += margin;
	rect.right -= margin;
	rect.bottom -= margin;
	
	mp_direct2d->DrawRoundedRectangle(rect, 5.0f);
}

void ButtonShape::DrawCircleShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::CIRCLE));
#endif 

	auto rect = m_buttonTable.at(TYPE::CIRCLE);
	const float margin = 7.0f;
	rect.left += margin;
	rect.top += margin;
	rect.right -= margin;
	rect.bottom -= margin;

	mp_direct2d->DrawEllipse(rect);
}

void ButtonShape::DrawTextShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::TEXT));
#endif 

	auto rect = m_buttonTable.at(TYPE::TEXT);
	auto prevTextFormat = mp_direct2d->SetTextFormat(mp_textFormat);
	mp_direct2d->DrawUserText(L"T", rect);
	mp_direct2d->SetTextFormat(prevTextFormat);
}

void ButtonShape::DrawStrokeShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::STROKE));
#endif 

	for (auto &rect : m_strokShapeRects) {
		mp_direct2d->DrawEllipse(rect);	
	}
}

void ButtonShape::DrawGradiationShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::GRADIATION));
#endif 

	ID2D1Brush *p_prevBrush = mp_direct2d->SetBrush(mp_gradientBrush);
	
	for (auto p_geometry : m_gradientGeometries) {
		mp_direct2d->DrawGeometry(p_geometry);
	}

	mp_direct2d->SetBrush(p_prevBrush);
}

void ButtonShape::DrawColorShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::COLOR));
#endif 

	// draw hue circle
	for (const auto &hueData : m_hueDataList) {
		mp_direct2d->SetBrushColor(hueData.color);
		mp_direct2d->DrawLine(hueData.point, hueData.point);
	}

	// draw selected color circle
	//if (!m_isGradationMode) 
	{
		auto rect = m_buttonTable.at(TYPE::COLOR);
		rect.left += m_colorShapeMargin;
		rect.top += m_colorShapeMargin;
		rect.right -= m_colorShapeMargin;
		rect.bottom -= m_colorShapeMargin;

		ID2D1Brush *const p_prevBrush = mp_direct2d->SetBrush(mp_colorShapeBrush);
		mp_direct2d->FillEllipse(rect);
		mp_direct2d->SetBrush(p_prevBrush);
	}
}

void ButtonShape::DrawFadeShape()
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(TYPE::FADE));
#endif 
	
	auto rect = m_buttonTable.at(TYPE::FADE);
	rect.left += m_fadeShapeMargin;
	rect.top += m_fadeShapeMargin;
	rect.right -= m_fadeShapeMargin;
	rect.bottom -= m_fadeShapeMargin;
	
	mp_direct2d->SetBrushColor(m_highlightColor);

	mp_direct2d->DrawEllipse(rect);
	for (const auto &rect : m_fadeShapeRects) {
		mp_direct2d->DrawLine({ rect.left, rect.top }, { rect.right,rect.bottom });
	}
}

void ButtonShape::InitCurveShapeData()
{
	auto p_factory = gp_appCore->GetFactory();
	mp_curveGeometry = nullptr;
	if (S_OK != p_factory->CreatePathGeometry(&mp_curveGeometry)) {
		return;
	}

	ID2D1GeometrySink *p_sink = nullptr;
	if (S_OK != mp_curveGeometry->Open(&p_sink)) {
		mp_curveGeometry->Release();
		mp_curveGeometry = nullptr;

		return;
	}

	auto InsertPointList = [](std::vector<DPoint> &postList, const unsigned int a_radius, const unsigned int a_startDegree, const DPoint &a_startPos)
	{
		const unsigned char POINT_COUNT = 32;
		double radian;

		size_t index;
		for (size_t count = 0; count < POINT_COUNT; count++) {
			index = postList.size();
			radian = (index * 5 + a_startDegree) * CONVERT_RADIAN;
			postList.push_back({ 
				a_startPos.x + index * 0.5f,
				a_startPos.y + static_cast<float>(sin(radian) * a_radius)
			});
		};
	};

	std::vector<DPoint> pointList;
	const auto rect = m_buttonTable.at(TYPE::CURVE);
	const float startPosX = rect.left + 6.0f;
	const float startPosY = rect.top + (rect.bottom - rect.top) * 0.7f;

	// set point of first curve
	InsertPointList(pointList, 17, 180, { startPosX, startPosY });
	// set point of second curve
	InsertPointList(pointList, 10, 240, { startPosX, startPosY - 12.0f });

	p_sink->BeginFigure(pointList[0], D2D1_FIGURE_BEGIN_FILLED);
	for (size_t i = 1; i < pointList.size(); i++) {
		p_sink->AddLine(pointList[i]);
	}
	p_sink->EndFigure(D2D1_FIGURE_END_OPEN);
	p_sink->Close();

	InterfaceRelease(&p_sink);
}

void ButtonShape::InitStrokyShapeData()
{
	m_strokShapeRects.resize(3);

	const auto rect = m_buttonTable.at(TYPE::STROKE);	
	const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
	const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;

	const float smallCircleOffset = 3.0f;
	const float smallCircleDiameter = 6.0f;
	const float circleDiameter = 8.0f;
	const float lageCircleDiameter = 10.0f;

	m_strokShapeRects[0] = { 
		rect.left + smallCircleOffset, centerPosY - smallCircleDiameter / 2.0f,
		rect.left + smallCircleOffset + smallCircleDiameter, centerPosY + smallCircleDiameter / 2.0f
	};
	m_strokShapeRects[1] = { 
		centerPosX - circleDiameter / 2.0f, centerPosY - circleDiameter / 2.0f,
		centerPosX + circleDiameter / 2.0f, centerPosY + circleDiameter / 2.0f
	};
	m_strokShapeRects[2] = { 
		rect.right - lageCircleDiameter, centerPosY - lageCircleDiameter / 2.0f,
		rect.right, centerPosY + lageCircleDiameter / 2.0f
	};
}

void ButtonShape::InitGradiationShapeData()
{
	///////////////////////////////////////////////////////////////////
	// init gradient brush
	///////////////////////////////////////////////////////////////////

	const size_t gradientCount = 3;
	// create gradient color to be used on hover
	D2D1_GRADIENT_STOP gradientStopList[gradientCount] = {
		{0.2f, RGB_TO_COLORF(RGB(161, 196, 253))},
		{0.5f, RGB_TO_COLORF(RGB(132, 250, 176))},
		{0.8f, RGB_TO_COLORF(RGB(252, 182, 159))},
	};

	const float margin = 7.0f;
	auto rect = m_buttonTable.at(TYPE::GRADIATION);
	rect.left += margin;
	rect.top += margin;
	rect.right -= margin;
	rect.bottom -= margin;
	const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
	const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;
	
	const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientData = {
		{rect.left , rect.top}, {rect.right , rect.bottom},
	};

	mp_gradientBrush = nullptr;
	mp_gradientBrush = mp_direct2d->CreateLinearGradientBrush(gradientStopList, gradientCount, &gradientData);

	// create darker gradient color to be used on selected
	//gradientStopList[0] = { 0.2f, RGB_TO_COLORF(RGB(141, 176, 233)) };
	//gradientStopList[1] = { 0.5f, RGB_TO_COLORF(RGB(112, 230, 156)) };
	//gradientStopList[2] = { 0.8f, RGB_TO_COLORF(RGB(232, 162, 139)) };
	//mp_darkGradientbrush = mp_d2dView->CreateLinearGradientBrush(gradientStopList, gradientCount, &gradientData);

	///////////////////////////////////////////////////////////////////
	// init geometry
	///////////////////////////////////////////////////////////////////

	auto OnFailedInit = [](std::vector<ID2D1PathGeometry *> &gradientGeometries)
	{
		for (auto &geometry : gradientGeometries) {
			geometry = nullptr;
		}
	};

	const size_t geometryCount = 3;
	m_gradientGeometries.resize(geometryCount);
	auto p_factory = gp_appCore->GetFactory();

	for (auto &geometry : m_gradientGeometries) {
		if (S_OK != p_factory->CreatePathGeometry(&geometry)) {
			OnFailedInit(m_gradientGeometries);
			return;
		}
	}

	std::vector<ID2D1GeometrySink *> sinkList(geometryCount);
	for (size_t i = 0; i < geometryCount; i++) {
		if (S_OK != m_gradientGeometries[i]->Open(&sinkList[i])) {
			OnFailedInit(m_gradientGeometries);
			break;
		}
	}
	
	const double radius = (rect.right - rect.left) / 2.5;
	int degreeList[geometryCount] = { 20, 140, 260 };
	double radian;

	size_t i = 0;
	for (auto sink : sinkList) {
		radian = degreeList[i] * CONVERT_RADIAN;
		sink->BeginFigure(
			{centerPosX + static_cast<float>(sin(radian) * radius), centerPosY + static_cast<float>(cos(radian) * radius)},
			D2D1_FIGURE_BEGIN_FILLED
		);

		for (int degree = 0; degree < 80; degree++) {
			degreeList[i]++;
			radian = degreeList[i] * CONVERT_RADIAN;

			sink->AddLine(
				{centerPosX + static_cast<float>(sin(radian) * radius), centerPosY + static_cast<float>(cos(radian) * radius)}
			);
		}
		sink->AddLine(
			{centerPosX + static_cast<float>(sin(radian) * radius * 0.5), centerPosY + static_cast<float>(cos(radian) * radius * 0.5)}
		);

		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		sink->Close();
		sink->Release();
		
		i++;
	}
}

void ButtonShape::InitColorShapeData()
{
	const double PI = 3.14159265358979;
	const float HUE_RADIUS = 14.0f;

	//----------------------------------------
	// init first the hue circle of color symbol
	//----------------------------------------
	m_hueDataList.resize(90);

	auto rect = m_buttonTable.at(TYPE::COLOR);
	const float marginOffset = 7.0f;
	rect.left += m_colorShapeMargin - marginOffset;
	rect.top += m_colorShapeMargin - marginOffset;
	rect.right -= m_colorShapeMargin - marginOffset;
	rect.bottom -= m_colorShapeMargin - marginOffset;

	const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
	const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;
	unsigned char degree = 0;
	double radian;

	for (auto &hueData : m_hueDataList) {
		radian = PI * degree / 45.0;

		hueData.point.x = static_cast<float>(centerPosX + HUE_RADIUS * cos(radian));
		hueData.point.y = static_cast<float>(centerPosY - HUE_RADIUS * sin(radian));
		// convert hue to [0,6]
		hueData.color = fromHueToColorF(degree / 15.0f);

		degree++;
	}

	//----------------------------------------
	// init the gradient color of color symbol
	//----------------------------------------
	m_selectedBrushColor = RGB_TO_COLORF(RED_400);
	UpdateColorSymbolBrush();
}

void ButtonShape::UpdateColorSymbolBrush()
{
	const unsigned int gradientCount = 3;
	const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientData = { {14.0f, 303.0f}, {42.0f, 275.0f} };
	D2D1_GRADIENT_STOP gradientStopList[gradientCount] = {
		{0.177777f, {0.0f, 0.0f, 0.0f, 1.0f}},
		{0.5f, m_selectedBrushColor},
		{0.833333f, { 1.0f, 1.0f, 1.0f, 1.0f }},
	};

	mp_colorShapeBrush = mp_direct2d->CreateLinearGradientBrush(gradientStopList, gradientCount, &gradientData);
}

void ButtonShape::InitFadeShapeData()
{
	
	auto rect = m_buttonTable.at(TYPE::FADE);
	rect.left += m_fadeShapeMargin;
	rect.top += m_fadeShapeMargin;
	rect.right -= m_fadeShapeMargin;
	rect.bottom -= m_fadeShapeMargin;

	const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
	const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;
	const float radius = (rect.right - rect.left) / 2.0f + 0.5f;

	DRect tempRect{ centerPosX, centerPosY - radius, centerPosX, centerPosY - radius - 5.0f };
	m_fadeShapeRects.push_back(tempRect);

	tempRect.left -= 3.0f;
	tempRect.top = tempRect.bottom;
	tempRect.right += 3.0f;
	m_fadeShapeRects.push_back(tempRect);

	double radian = 145 * CONVERT_RADIAN;
	m_fadeShapeRects.push_back({
		centerPosX + static_cast<float>(sin(radian) * radius), centerPosY + static_cast<float>(cos(radian) * radius),
		centerPosX + static_cast<float>(sin(radian) * radius * 1.2f), centerPosY + static_cast<float>(cos(radian) * radius * 1.2f)
	});

	m_fadeShapeRects.push_back({
		centerPosX, centerPosY,
		centerPosX, centerPosY - radius + 7.0f
	});

	radian = 60 * CONVERT_RADIAN;
	m_fadeShapeRects.push_back({
		centerPosX, centerPosY,
		centerPosX + static_cast<float>(sin(radian) * radius * 0.6f), centerPosY + static_cast<float>(cos(radian) * radius * 0.6f)
	});
}