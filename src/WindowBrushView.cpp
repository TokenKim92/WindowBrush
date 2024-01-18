#include "WindowBrushView.h"
#include "ColorPalette.h"
#include "Utility.h"

//#define SHOW_BUTTON_AREA

extern ApplicationCore *gp_appCore;

WindowBrushView::WindowBrushView(const HWND &ah_window, const CM &a_mode, const RECT *const ap_viewRect) :
	Direct2DEx(ah_window, ap_viewRect),
	m_colorShapeMargin(12.0f),
	m_fadeShapeMargin(10.0f)
{
	mp_textFormat = nullptr;
	SetColorMode(a_mode);

	mp_curveGeometry = nullptr;
	mp_gradientBrush = nullptr;
	mp_colorShapeBrush = nullptr;

	m_drawTable.insert({ WINDOW_BRUSH::BT::CURVE, &WindowBrushView::DrawCurveShape });
	m_drawTable.insert({ WINDOW_BRUSH::BT::RECTANGLE, &WindowBrushView::DrawRectangleShape });
	m_drawTable.insert({ WINDOW_BRUSH::BT::CIRCLE, &WindowBrushView::DrawCircleShape });
	m_drawTable.insert({ WINDOW_BRUSH::BT::TEXT, &WindowBrushView::DrawTextShape });
	m_drawTable.insert({ WINDOW_BRUSH::BT::STROKE, &WindowBrushView::DrawStrokeShape });
	m_drawTable.insert({ WINDOW_BRUSH::BT::GRADIENT, &WindowBrushView::DrawGradientShape });
	m_drawTable.insert({ WINDOW_BRUSH::BT::COLOR, &WindowBrushView::DrawColorShape });
	m_drawTable.insert({ WINDOW_BRUSH::BT::FADE, &WindowBrushView::DrawFadeShape });
}

WindowBrushView::~WindowBrushView()
{
	InterfaceRelease(&mp_textFormat);
	InterfaceRelease(&mp_curveGeometry);
	InterfaceRelease(&mp_gradientBrush);
	for (auto &geometry : m_gradientGeometries) {
		InterfaceRelease(&geometry);
	}
	InterfaceRelease(&mp_colorShapeBrush);
}

int WindowBrushView::Create()
{
	const auto InitButtonRects = [](WindowBrushView *const ap_view)
	{
		const RECT &viewRect = *ap_view->mp_viewRect;
		const float buttonSize = viewRect.right - viewRect.left - WINDOW_BRUSH::BUTTON_X_MARGIN * 2.0f;
		const size_t buttonCount = 8;

		std::vector<WINDOW_BRUSH::BT> buttonShapeList = {
			WINDOW_BRUSH::BT::CURVE,
			WINDOW_BRUSH::BT::RECTANGLE,
			WINDOW_BRUSH::BT::CIRCLE,
			WINDOW_BRUSH::BT::TEXT,
			WINDOW_BRUSH::BT::STROKE,
			WINDOW_BRUSH::BT::GRADIENT,
			WINDOW_BRUSH::BT::COLOR,
			WINDOW_BRUSH::BT::FADE
		};

		size_t index = 0;
		for (const auto &tpye : buttonShapeList) {
			ap_view->m_buttonTable.insert({
				tpye,
				{ 
					WINDOW_BRUSH::BUTTON_X_MARGIN, buttonSize * index, 
					viewRect.right - WINDOW_BRUSH::BUTTON_X_MARGIN, buttonSize * (index + 1)
				}
				});

			index++;
		}

		const float fadeRectOffet = 3.0f;
		DRect &rect = ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::FADE);
		rect.top += fadeRectOffet;
		rect.bottom += fadeRectOffet;
	};
	const auto InitDivider = [](WindowBrushView *const ap_view)
	{
		const auto AddDividerRect = [](std::vector<DRect> &a_divierList, const DRect &a_rect)
		{
			a_divierList.push_back(DRect({ a_rect.left, a_rect.bottom, a_rect.right, a_rect.bottom }));
		};

		AddDividerRect(ap_view->m_dividerList, ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::TEXT));
		AddDividerRect(ap_view->m_dividerList, ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::STROKE));
		AddDividerRect(ap_view->m_dividerList, ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::COLOR));
	};
	const auto InitCurveShapeData = [](WindowBrushView *const ap_view)
	{
		auto p_factory = gp_appCore->GetFactory();
		if (S_OK != p_factory->CreatePathGeometry(&ap_view->mp_curveGeometry)) {
			return;
		}

		ID2D1GeometrySink *p_sink = nullptr;
		if (S_OK != ap_view->mp_curveGeometry->Open(&p_sink)) {
			ap_view->mp_curveGeometry->Release();
			ap_view->mp_curveGeometry = nullptr;

			return;
		}

		const auto InsertPointList = [](std::vector<DPoint> &postList, const unsigned int a_radius, const unsigned int a_startDegree, const DPoint &a_startPos)
		{
			const unsigned char POINT_COUNT = 32;
			double radian;

			size_t index;
			for (size_t count = 0; count < POINT_COUNT; count++) {
				index = postList.size();
				radian = (index * 5 + a_startDegree) * WINDOW_BRUSH::CONVERT_RADIAN;
				postList.push_back({
					a_startPos.x + index * 0.5f,
					a_startPos.y + static_cast<float>(sin(radian) * a_radius)
					});
			};
		};

		std::vector<DPoint> pointList;
		const auto rect = ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::CURVE);
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
	};
	const auto InitStrokyShapeData = [](WindowBrushView *const ap_view)
	{
		ap_view->m_strokeShapeRects.resize(3);

		const auto rect = ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::STROKE);
		const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
		const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;

		const float smallCircleOffset = 3.0f;
		const float smallCircleDiameter = 6.0f;
		const float circleDiameter = 8.0f;
		const float lageCircleDiameter = 10.0f;

		ap_view->m_strokeShapeRects[0] = {
			rect.left + smallCircleOffset, centerPosY - smallCircleDiameter / 2.0f,
			rect.left + smallCircleOffset + smallCircleDiameter, centerPosY + smallCircleDiameter / 2.0f
		};
		ap_view->m_strokeShapeRects[1] = {
			centerPosX - circleDiameter / 2.0f, centerPosY - circleDiameter / 2.0f,
			centerPosX + circleDiameter / 2.0f, centerPosY + circleDiameter / 2.0f
		};
		ap_view->m_strokeShapeRects[2] = {
			rect.right - lageCircleDiameter, centerPosY - lageCircleDiameter / 2.0f,
			rect.right, centerPosY + lageCircleDiameter / 2.0f
		};
	};
	const auto InitGradiationShapeData = [](WindowBrushView *const ap_view)
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

		auto rect = ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::GRADIENT);
		ShrinkRect(rect, 7.0f);
		const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
		const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;

		const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientData = {
			{rect.left , rect.top}, {rect.right , rect.bottom},
		};

		ap_view->mp_gradientBrush = ap_view->CreateLinearGradientBrush(gradientStopList, gradientCount, &gradientData);

		///////////////////////////////////////////////////////////////////
		// init geometry
		///////////////////////////////////////////////////////////////////

		const auto OnFailedInit = [](std::vector<ID2D1PathGeometry *> &gradientGeometries)
		{
			for (auto &geometry : gradientGeometries) {
				geometry = nullptr;
			}
		};

		const size_t geometryCount = 3;
		ap_view->m_gradientGeometries.resize(geometryCount);
		auto p_factory = gp_appCore->GetFactory();

		for (auto &geometry : ap_view->m_gradientGeometries) {
			if (S_OK != p_factory->CreatePathGeometry(&geometry)) {
				OnFailedInit(ap_view->m_gradientGeometries);
				return;
			}
		}

		std::vector<ID2D1GeometrySink *> sinkList(geometryCount);
		for (size_t i = 0; i < geometryCount; i++) {
			if (S_OK != ap_view->m_gradientGeometries[i]->Open(&sinkList[i])) {
				OnFailedInit(ap_view->m_gradientGeometries);
				break;
			}
		}

		const double radius = (rect.right - rect.left) / 2.5;
		int degreeList[geometryCount] = { 20, 140, 260 };
		double radian;

		size_t i = 0;
		for (auto sink : sinkList) {
			radian = degreeList[i] * WINDOW_BRUSH::CONVERT_RADIAN;
			sink->BeginFigure(
				{ centerPosX + static_cast<float>(sin(radian) * radius), centerPosY + static_cast<float>(cos(radian) * radius) },
				D2D1_FIGURE_BEGIN_FILLED
			);

			for (int degree = 0; degree < 80; degree++) {
				degreeList[i]++;
				radian = degreeList[i] * WINDOW_BRUSH::CONVERT_RADIAN;

				sink->AddLine(
					{ centerPosX + static_cast<float>(sin(radian) * radius), centerPosY + static_cast<float>(cos(radian) * radius) }
				);
			}
			sink->AddLine(
				{ centerPosX + static_cast<float>(sin(radian) * radius * 0.5), centerPosY + static_cast<float>(cos(radian) * radius * 0.5) }
			);

			sink->EndFigure(D2D1_FIGURE_END_OPEN);
			sink->Close();
			sink->Release();

			i++;
		}
	};
	const auto InitColorShapeData = [](WindowBrushView *const ap_view)
	{
		const float HUE_RADIUS = 14.0f;

		//----------------------------------------
		// init first the hue circle of color symbol
		//----------------------------------------
		ap_view->m_hueDataList.resize(90);

		auto rect = ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::COLOR);
		ShrinkRect(rect, ap_view->m_colorShapeMargin - 7.0f);

		const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
		const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;
		unsigned char degree = 0;
		double radian;

		for (auto &hueData : ap_view->m_hueDataList) {
			radian = PI * degree / 45.0;

			hueData.second.x = static_cast<float>(centerPosX + HUE_RADIUS * cos(radian));
			hueData.second.y = static_cast<float>(centerPosY - HUE_RADIUS * sin(radian));
			// convert hue to [0,6]
			hueData.first = FromHueToColor(degree / 15.0f);
			hueData.first.a = WINDOW_BRUSH::DEFAULT_TRANSPARENCY;

			degree++;
		}
	};
	const auto InitFadeShapeData = [](WindowBrushView *const ap_view)
	{

		auto rect = ap_view->m_buttonTable.at(WINDOW_BRUSH::BT::FADE);
		ShrinkRect(rect, ap_view->m_fadeShapeMargin);

		const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
		const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;
		const float radius = (rect.right - rect.left) / 2.0f + 0.5f;

		DRect tempRect{ centerPosX, centerPosY - radius, centerPosX, centerPosY - radius - 5.0f };
		ap_view->m_fadeShapeRects.push_back(tempRect);

		tempRect.left -= 3.0f;
		tempRect.top = tempRect.bottom;
		tempRect.right += 3.0f;
		ap_view->m_fadeShapeRects.push_back(tempRect);

		double radian = 145 * WINDOW_BRUSH::CONVERT_RADIAN;
		ap_view->m_fadeShapeRects.push_back({
			centerPosX + static_cast<float>(sin(radian) * radius), centerPosY + static_cast<float>(cos(radian) * radius),
			centerPosX + static_cast<float>(sin(radian) * radius * 1.2f), centerPosY + static_cast<float>(cos(radian) * radius * 1.2f)
			});

		ap_view->m_fadeShapeRects.push_back({
			centerPosX, centerPosY,
			centerPosX, centerPosY - radius + 7.0f
			});

		radian = 60 * WINDOW_BRUSH::CONVERT_RADIAN;
		ap_view->m_fadeShapeRects.push_back({
			centerPosX, centerPosY,
			centerPosX + static_cast<float>(sin(radian) * radius * 0.6f), centerPosY + static_cast<float>(cos(radian) * radius * 0.6f)
			});
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	auto result = ::Direct2D::Create();
	if (S_OK != result) {
		return result;
	}

	InitButtonRects(this);
	InitDivider(this);

	InitCurveShapeData(this);
	InitStrokyShapeData(this);
	InitGradiationShapeData(this);
	InitColorShapeData(this);
	InitFadeShapeData(this);

	mp_textFormat = CreateTextFormat(L"Times New Roman", 30.0f, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL);
	mp_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	SetStrokeWidth(2.5f);

	return S_OK;
}

void WindowBrushView::Paint(const WINDOW_BRUSH::MD &a_modelData)
{
	Clear();

	for (auto const &[type, drawFunction] : m_drawTable) {
		(this->*drawFunction)(a_modelData);
	}

	SetBrushColor(RGB_TO_COLORF(NEUTRAL_300));
	for (auto const &divier : m_dividerList) {
		DrawRectangle(divier);
	}
}

void WindowBrushView::UpdateTextColorOnHover(const WINDOW_BRUSH::BT &a_type, const WINDOW_BRUSH::MD &a_data)
{
	DColor color = a_type == a_data.drawMode
		? m_highlightColor
		: m_textColor;
	if (a_type == a_data.hoverButtonType) {
		color.a = 1.0f;
	}
	SetBrushColor(color);
}

void WindowBrushView::DrawCurveShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::CURVE));
#endif 

	if (nullptr != mp_curveGeometry) {
		const auto rect = m_buttonTable.at(WINDOW_BRUSH::BT::CURVE);
		const float centerPosX = rect.left + (rect.right - rect.left) / 2.0f;
		const float centerPosY = rect.top + (rect.bottom - rect.top) / 2.0f;

		UpdateTextColorOnHover(WINDOW_BRUSH::BT::CURVE, a_data);

		SetMatrixTransform(D2D1::Matrix3x2F::Rotation(10, { centerPosX, centerPosY }));
		DrawGeometry(mp_curveGeometry);
		SetMatrixTransform(D2D1::Matrix3x2F::Rotation(0, { centerPosX, centerPosY }));
	}
}

void WindowBrushView::DrawRectangleShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::RECTANGLE));
#endif 

	auto rect = m_buttonTable.at(WINDOW_BRUSH::BT::RECTANGLE);
	ShrinkRect(rect, 7.0f);

	UpdateTextColorOnHover(WINDOW_BRUSH::BT::RECTANGLE, a_data);

	DrawRoundedRectangle(rect, 5.0f);
}

void WindowBrushView::DrawCircleShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::CIRCLE));
#endif 

	auto rect = m_buttonTable.at(WINDOW_BRUSH::BT::CIRCLE);
	ShrinkRect(rect, 7.0f);

	UpdateTextColorOnHover(WINDOW_BRUSH::BT::CIRCLE, a_data);

	DrawEllipse(rect);
}

void WindowBrushView::DrawTextShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::TEXT));
#endif 

	auto rect = m_buttonTable.at(WINDOW_BRUSH::BT::TEXT);

	UpdateTextColorOnHover(WINDOW_BRUSH::BT::TEXT, a_data);

	auto prevTextFormat = SetTextFormat(mp_textFormat);
	DrawUserText(L"T", rect);
	SetTextFormat(prevTextFormat);
}

void WindowBrushView::DrawStrokeShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::STROKE));
#endif 

	DColor color = m_textColor;
	if (WINDOW_BRUSH::BT::STROKE == a_data.hoverButtonType) {
		color.a = 1.0f;
	}
	SetBrushColor(color);
	DrawEllipse(m_strokeShapeRects.at(0));
	DrawEllipse(m_strokeShapeRects.at(2));

	color = m_highlightColor;
	if (WINDOW_BRUSH::BT::STROKE == a_data.hoverButtonType) {
		color.a = 1.0f;
	}
	SetBrushColor(color);
	DrawEllipse(m_strokeShapeRects.at(1));
}

void WindowBrushView::DrawGradientShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::GRADIATION));
#endif 
	// able gradation button
	if (a_data.isGradientMode && nullptr != mp_gradientBrush) {
		ID2D1Brush *p_prevBrush = SetBrush(mp_gradientBrush);
		const float transparency = WINDOW_BRUSH::BT::GRADIENT == a_data.hoverButtonType
			? 1.0f
			: WINDOW_BRUSH::DEFAULT_TRANSPARENCY;
		mp_gradientBrush->SetOpacity(transparency);

		for (auto p_geometry : m_gradientGeometries) {
			DrawGeometry(p_geometry);
		}

		SetBrush(p_prevBrush);

		return;
	}

	// disable gradation button
	UpdateTextColorOnHover(WINDOW_BRUSH::BT::GRADIENT, a_data);

	for (auto p_geometry : m_gradientGeometries) {
		DrawGeometry(p_geometry);
	}
}

void WindowBrushView::DrawColorShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::COLOR));
#endif 

	// draw hue circle
	DColor color;
	for (const auto &hueData : m_hueDataList) {
		color = hueData.first;
		if (WINDOW_BRUSH::BT::COLOR == a_data.hoverButtonType) {
			color.a = 1.0f;
		}
		SetBrushColor(color);
		DrawLine(hueData.second, hueData.second);
	}

	// able color button
	// draw selected color circle
	if (!a_data.isGradientMode && nullptr != mp_colorShapeBrush) {
		auto rect = m_buttonTable.at(WINDOW_BRUSH::BT::COLOR);
		ShrinkRect(rect, m_colorShapeMargin);

		ID2D1Brush *const p_prevBrush = SetBrush(mp_colorShapeBrush);
		const float transparency = WINDOW_BRUSH::BT::COLOR == a_data.hoverButtonType
			? 1.0f
			: WINDOW_BRUSH::DEFAULT_TRANSPARENCY;
		mp_colorShapeBrush->SetOpacity(transparency);

		FillEllipse(rect);
		SetBrush(p_prevBrush);
	}
}

void WindowBrushView::DrawFadeShape(const WINDOW_BRUSH::MD &a_data)
{
#ifdef  SHOW_BUTTON_AREA
	mp_direct2d->DrawRectangle(m_buttonTable.at(WINDOW_BRUSH::BT::FADE));
#endif 

	auto rect = m_buttonTable.at(WINDOW_BRUSH::BT::FADE);
	ShrinkRect(rect, m_fadeShapeMargin);

	DColor color = a_data.isFadeMode
		? m_highlightColor
		: m_textColor;
	if (WINDOW_BRUSH::BT::FADE == a_data.hoverButtonType) {
		color.a = 1.0f;
	}
	SetBrushColor(color);

	DrawEllipse(rect);
	for (const auto &rect : m_fadeShapeRects) {
		DrawLine({ rect.left, rect.top }, { rect.right,rect.bottom });
	}
}

const std::map<WINDOW_BRUSH::BT, DRect> WindowBrushView::GetButtonTable()
{
	return m_buttonTable;
}

void WindowBrushView::SetColorMode(const CM &a_mode)
{
	if (CM::LIGHT == a_mode) {
		m_textColor = RGB_TO_COLORF(NEUTRAL_600);
		m_highlightColor = RGB_TO_COLORF(ORANGE_400);
		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_100));
	}
	else {
		m_textColor = RGB_TO_COLORF(NEUTRAL_200);
		m_highlightColor = RGB_TO_COLORF(VIOLET_600);
		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_800));
	}

	m_textColor.a = WINDOW_BRUSH::DEFAULT_TRANSPARENCY;
	m_highlightColor.a = WINDOW_BRUSH::DEFAULT_TRANSPARENCY;
}

void WindowBrushView::UpdateColorSymbolBrush(const DColor &a_color)
{
	// update the gradient color of color symbol
	const unsigned int gradientCount = 3;
	const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientData = { {14.0f, 303.0f}, {42.0f, 275.0f} };
	D2D1_GRADIENT_STOP gradientStopList[gradientCount] = {
		{0.177777f, {0.0f, 0.0f, 0.0f, 1.0f}},
		{0.5f, a_color},
		{0.833333f, { 1.0f, 1.0f, 1.0f, 1.0f }},
	};

	if (nullptr != mp_colorShapeBrush) {
		InterfaceRelease(&mp_colorShapeBrush);
	}
	mp_colorShapeBrush = CreateLinearGradientBrush(gradientStopList, gradientCount, &gradientData);
}
