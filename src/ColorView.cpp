#include "ColorView.h"
#include "Utility.h"
#include "ColorPalette.h"

extern ApplicationCore *gp_appCore;

ColorView::ColorView(
	const HWND ah_window, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList,
	const CM &a_mode, const RECT *const ap_viewRect
) :
	Direct2DEx(ah_window, ap_viewRect),
	m_defaultTransparency(0.6f),
	m_colorList(a_colorList)
{
	memset(&m_textRect, 0, sizeof(DRect));
	if (CM::DARK == a_mode) {
		m_titleColor = RGB_TO_COLORF(NEUTRAL_200);
		m_textBackgroundColor = RGB_TO_COLORF(NEUTRAL_900);
		m_borderColor = RGB_TO_COLORF(NEUTRAL_300);

		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_800));
	}
	else {
		m_titleColor = RGB_TO_COLORF(NEUTRAL_700);
		m_textBackgroundColor = RGB_TO_COLORF(NEUTRAL_200);
		m_borderColor = RGB_TO_COLORF(NEUTRAL_500);

		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_50));
	}

	mp_titleFont = nullptr;
	mp_addButtonStroke = nullptr;

	m_selectedColorData.second = a_selectedColor;
	m_colorCountPerWidth = 0;
	m_colorCountPerHeight = 0;
	memset(&m_colorCircleStartPoint, 0, sizeof(DPoint));
	m_maxColorDataSize = 0;

	m_selectedHue = RGB_TO_COLORF((COLORREF)0x0100e3);
	m_selectedLightness = m_selectedHue;
	memset(&m_lightnessRect, 0, sizeof(DRect));

	mp_lightnessGradientBrush = nullptr;

	mp_memoryBitmap = nullptr;
	mp_memoryTarget = nullptr;
}

ColorView::~ColorView()
{
	InterfaceRelease(&mp_titleFont);
	InterfaceRelease(&mp_addButtonStroke);

	InterfaceRelease(&mp_lightnessGradientBrush);
	InterfaceRelease(&mp_memoryBitmap);
	InterfaceRelease(&mp_memoryTarget);
}

int ColorView::Create()
{
	auto result = ::Direct2D::Create();
	if (S_OK != result) {
		return result;
	}

	m_viewSize = { mp_viewRect->right - mp_viewRect->left, mp_viewRect->bottom - mp_viewRect->top };

	InitSelectMode();

	// create instance of direct2d
	mp_titleFont = CreateTextFormat(DEFAULT_FONT_NAME, 14.0f, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_titleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_titleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	mp_addButtonStroke = CreateUserStrokeStyle(D2D1_DASH_STYLE_DASH);

	return S_OK;
}

DColor ColorView::GetColor(const size_t &a_index)
{
	if (INVALID_INDEX != a_index || m_colorDataTable.size() > a_index) {
		return m_colorDataTable.at(a_index).first;
	}

	return DColor({ 0.0f, 0.0f, 0.0f, 1.0f });
}

const std::map<size_t, DRect> ColorView::GetColorDataTable()
{
	std::map<size_t, DRect> tempMap;

	for (auto &[index, data] : m_colorDataTable) {
		tempMap.insert({ index, data.second });
	}

	return tempMap;
}

const std::pair<size_t, DRect> &ColorView::GetAddButtonData()
{
	return m_addButtonData;
}

const std::map<CBT, DRect> &ColorView::GetButtonTable()
{
	return m_buttonTable;
}

void ColorView::InitSelectMode()
{
	m_textRect = { 0.0f, 0.0f, static_cast<float>(m_viewSize.cx), static_cast<float>(TEXT_HEIGHT) };

	const int height = m_viewSize.cy - TEXT_HEIGHT;
	size_t offset = m_viewSize.cx % INTERVAL ? 0 : 1;
	m_colorCountPerWidth = m_viewSize.cx / INTERVAL - offset;
	offset = height % INTERVAL ? 0 : 1;
	m_colorCountPerHeight = height / INTERVAL - offset;

	m_colorCircleStartPoint = {
		(m_viewSize.cx - (m_colorCountPerWidth - 1) * INTERVAL) / 2.0f,
		TEXT_HEIGHT + (height - (m_colorCountPerHeight - 1) * INTERVAL) / 2.0f
	};

	m_maxColorDataSize = m_colorCountPerWidth * m_colorCountPerHeight;

	// init color data about color and rect
	const std::vector<DColor> defaultColorList({
		RGB_TO_COLORF(NEUTRAL_950), RGB_TO_COLORF(NEUTRAL_500), RGB_TO_COLORF(NEUTRAL_50), RGB_TO_COLORF(RED_500), RGB_TO_COLORF(ORANGE_500),
		RGB_TO_COLORF(YELLOW_500), RGB_TO_COLORF(GREEN_500), RGB_TO_COLORF(BLUE_500), RGB_TO_COLORF(PURPLE_500), RGB_TO_COLORF(PINK_500)
		});
	std::vector<DColor> tempColorList = 0 == m_colorList.size()
		? defaultColorList
		: m_colorList;
	if (tempColorList.size() >= m_maxColorDataSize) {
		tempColorList.resize(m_maxColorDataSize - 1); // -1 for add button
	}

	size_t i = 0;
	for (const auto &color : tempColorList) {
		m_colorDataTable.insert({ i, {color, GetColorRect(i)} });
		i++;
	}

	// init add button rect
	m_addButtonData = { m_colorDataTable.size() , GetColorRect(m_colorDataTable.size()) };

	for (const auto &[index, colorData] : m_colorDataTable) {
		if (IsSameColor(colorData.first, m_selectedColorData.second)) {
			m_selectedColorData.first = index;
			break;
		}
	}
}

const DRect ColorView::GetColorRect(const size_t a_index)
{
	const float COLOR_RADIUS = 10.0f;
	const float posX = static_cast<float>(m_colorCircleStartPoint.x + INTERVAL * (a_index % m_colorCountPerWidth));
	const float posY = static_cast<float>(m_colorCircleStartPoint.y + INTERVAL * (a_index / m_colorCountPerHeight));

	return DRect({ posX - COLOR_RADIUS, posY - COLOR_RADIUS, posX + COLOR_RADIUS, posY + COLOR_RADIUS });
}

void ColorView::InitAddMode()
{
	const auto InitHueDataList = [](
		const float a_radius, const float &a_centerPosX, const float &a_centerPosY,
		std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> &a_hueDataList
		)
	{
		const float STROKE_WIDTH = 2.5f;
		const float startHueRadius = a_radius - STROKE_WIDTH;
		const float endHueRadius = a_radius + STROKE_WIDTH;

		a_hueDataList.resize(720);

		double radian;
		unsigned int degree = 0;
		for (auto &hueData : a_hueDataList) {
			radian = PI * degree / 360;

			// set strat point
			hueData.second.first = {
				static_cast<float>(a_centerPosX + startHueRadius * cos(radian)),
				static_cast<float>(a_centerPosY + startHueRadius * sin(radian))
			};
			// set end point
			hueData.second.second = {
				static_cast<float>(a_centerPosX + endHueRadius * cos(radian)),
				static_cast<float>(a_centerPosY + endHueRadius * sin(radian))
			};
			// set color
			hueData.first = FromHueToColor(degree / 120.0f);;

			degree++;
		}
	};

	const auto UpdateMemoryHueCircle = [](ID2D1RenderTarget *const ap_memoryTarget, std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> &a_hueDataList)
	{
		ID2D1SolidColorBrush *p_solidBrush;
		auto result = ap_memoryTarget->CreateSolidColorBrush(DColor({ 0.0f, 0.0f, 0.0f, 1.0f }), &p_solidBrush);
		if (S_OK != result) {
			return;
		}

		ap_memoryTarget->BeginDraw();

		for (const auto &hueData : a_hueDataList) {
			p_solidBrush->SetColor(hueData.first);
			ap_memoryTarget->DrawLine(hueData.second.first, hueData.second.second, p_solidBrush);
		}

		ap_memoryTarget->EndDraw();

		p_solidBrush->Release();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const int height = m_viewSize.cy - TEXT_HEIGHT - INDICATE_HEIGHT;
	const float radius = m_viewSize.cx * 0.2f;
	const float centerPosX = mp_viewRect->left + m_viewSize.cx / 2.0f;
	const float centerPosY = TEXT_HEIGHT + mp_viewRect->top + height / 2.0f;

	m_lightnessRect = {
		centerPosX - radius, centerPosY - radius,
		centerPosX + radius, centerPosY + radius
	};

	auto result = gp_appCore->GetWICFactory()->CreateBitmap(
		m_viewSize.cx, m_viewSize.cy, GUID_WICPixelFormat32bppPRGBA, WICBitmapCacheOnDemand, &mp_memoryBitmap
	);
	if (S_OK != result) {
		return;
	}

	result = gp_appCore->GetFactory()->CreateWicBitmapRenderTarget(mp_memoryBitmap, D2D1::RenderTargetProperties(), &mp_memoryTarget);
	if (S_OK != result) {
		return;
	}

	InitHueDataList(m_viewSize.cx * 0.33f, centerPosX, centerPosY, m_hueDataList);
	UpdateMemoryHueCircle(mp_memoryTarget, m_hueDataList);
	UpdateLightnessData(m_selectedHue);


	m_returnIconPoints = {
		{{ 14.0f, TEXT_HEIGHT / 2.0f }, { TEXT_HEIGHT - 14.0f, 10.0f }},
		{{ 14.0f, TEXT_HEIGHT / 2.0f }, { TEXT_HEIGHT - 14.0f, TEXT_HEIGHT - 10.0f }}
	};

	m_buttonTable.insert({ CBT::RETURN, { 10.0f, 10.0f, TEXT_HEIGHT - 10.0f, TEXT_HEIGHT - 10.0f } });

	return;
}

void ColorView::UpdateLightnessData(const DColor &a_hue)
{
	////////////////////////////////////////////////////////////////
	// update memory render target
	////////////////////////////////////////////////////////////////
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientData = {
		{m_lightnessRect.left, m_lightnessRect.bottom},
		{m_lightnessRect.right, m_lightnessRect.top}
	};

	const unsigned int gradientStopCount = 3;
	D2D1_GRADIENT_STOP p_gradientStopList[gradientStopCount] = {
		{ 0.177777f, {0.0f, 0.0f, 0.0f, 1.0f} },
		{ 0.5f, a_hue },
		{ 0.833333f, { 1.0f, 1.0f, 1.0f, 1.0f } }
	};

	ID2D1GradientStopCollection *p_gradientStop;
	auto result = mp_memoryTarget->CreateGradientStopCollection(p_gradientStopList, gradientStopCount, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &p_gradientStop);
	if (S_OK != result) {
		return;
	}

	ID2D1LinearGradientBrush *p_lineGradientBrush;
	result = mp_memoryTarget->CreateLinearGradientBrush(gradientData, p_gradientStop, &p_lineGradientBrush);
	if (S_OK != result) {
		p_gradientStop->Release();

		return;
	}

	const float radius = m_viewSize.cx / 2.0f;

	mp_memoryTarget->BeginDraw();
	// draw lightness circle
	D2D1_ELLIPSE ellipse = { {m_lightnessRect.left + radius, m_lightnessRect.top + radius}, radius, radius };
	mp_memoryTarget->FillEllipse(&ellipse, p_lineGradientBrush);
	mp_memoryTarget->EndDraw();

	p_lineGradientBrush->Release();
	p_gradientStop->Release();

	////////////////////////////////////////////////////////////////
	// update render target
	////////////////////////////////////////////////////////////////
	mp_lightnessGradientBrush = CreateLinearGradientBrush(p_gradientStopList, gradientStopCount, &gradientData);

	////////////////////////////////////////////////////////////////
	// update memory pattern
	////////////////////////////////////////////////////////////////
	WICRect wicRect = {
		static_cast<int>(mp_viewRect->left), static_cast<int>(mp_viewRect->top),
		static_cast<int>(m_viewSize.cx), static_cast<int>(m_viewSize.cy - TEXT_HEIGHT - INDICATE_HEIGHT)
	};
	IWICBitmapLock *p_lock = nullptr;

	if (nullptr == mp_memoryBitmap) {
		return;
	}

	if (S_OK == mp_memoryBitmap->Lock(&wicRect, WICBITMAPLOCKFLAGS_FORCE_DWORD, &p_lock)) {
		unsigned int bufferSize = 0;
		unsigned int stride = 0;
		unsigned char *p_pattern = nullptr;

		if (S_OK == p_lock->GetStride(&stride)) {
			if (S_OK == p_lock->GetDataPointer(&bufferSize, &p_pattern)) {
				if (p_pattern) {
					mp_memoryPattern = std::make_unique<unsigned char[]>(bufferSize);
					memcpy(mp_memoryPattern.get(), p_pattern, bufferSize);
				}
			}
		}
		p_lock->Release();
	}
}

void ColorView::Paint(const CDM &a_drawModw, const CMD &a_modelData)
{
	if (CDM::SELECT == a_drawModw) {
		PaintOnSelectMode(a_modelData);
	}
	else {
		PaintOnAddMode(a_modelData);
	}
}

void ColorView::PaintOnSelectMode(const CMD &a_modelData)
{
	DrawTitle(CDM::SELECT);

	SetStrokeWidth(2.0f);

	// draw all color
	DRect rect;
	for (auto const &[index, colorData] : m_colorDataTable) {
		rect = colorData.second;
		// draw a large circle where the mouse is located
		if (index == a_modelData.clickedIndex) {
			ShrinkRect(rect, 1.0f);
		}
		else if (index == a_modelData.hoverIndex) {
			ExpandRect(rect, 2.0f);
		}

		SetBrushColor(m_borderColor);
		DrawEllipse(rect);
		SetBrushColor(colorData.first);
		FillEllipse(rect);
	}

	// draw selected color
	const size_t selectedColorIndex = m_selectedColorData.first;
	if (INVALID_INDEX != selectedColorIndex) {
		rect = m_colorDataTable.at(selectedColorIndex).second;

		// draw border
		const bool isClicked = selectedColorIndex == a_modelData.clickedIndex;
		float offset = isClicked ? 2.0f : 4.0f;
		ExpandRect(rect, offset);
		SetBrushColor(m_borderColor);
		FillEllipse(rect);

		// draw color circle
		offset = !isClicked && selectedColorIndex == a_modelData.hoverIndex ? 2.0f : 4.0f;
		ShrinkRect(rect, offset);
		SetBrushColor(m_selectedColorData.second);
		FillEllipse(rect);
	}

	SetStrokeWidth(1.0f);

	DrawAddButton(a_modelData);
}

void ColorView::PaintOnAddMode(const CMD &a_modelData)
{
	DrawTitle(CDM::ADD);

	// draw return button
	DColor color = m_titleColor;
	if (CBT::RETURN == a_modelData.clickedButton || CBT::RETURN != a_modelData.hoverButton) {
		color.a = m_defaultTransparency;
	}
	SetBrushColor(color);
	SetStrokeWidth(3.0f);
	DrawLine(m_returnIconPoints[0].first, m_returnIconPoints[0].second);
	DrawLine(m_returnIconPoints[1].first, m_returnIconPoints[1].second);
	SetStrokeWidth(1.0f);

	DrawHueCircle();
	DrawLightnessCircle();
}

void ColorView::DrawTitle(const CDM &a_mode)
{
	const std::wstring title = CDM::SELECT == a_mode
		? L"Select Color"
		: L"Add Color";

	// draw background
	SetBrushColor(m_textBackgroundColor);
	FillRectangle(m_textRect);
	// draw title

	if (nullptr != mp_titleFont) {
		auto prevTextFormat = SetTextFormat(mp_titleFont);
		SetBrushColor(m_titleColor);
		DrawUserText(title.c_str(), m_textRect);
		SetTextFormat(prevTextFormat);
	}
}


void ColorView::DrawAddButton(const CMD &a_modelData)
{
	// draw main circle
	DRect mainRect = m_addButtonData.second;
	if (m_addButtonData.first == a_modelData.clickedIndex) {
		ShrinkRect(mainRect, 1.0f);
	}
	else if (m_addButtonData.first == a_modelData.hoverIndex) {
		ExpandRect(mainRect, 2.0f);
	}

	if (nullptr != mp_addButtonStroke) {
		ID2D1StrokeStyle *p_prevStrokeStyle = SetStrokeStyle(mp_addButtonStroke);
		SetBrushColor(m_titleColor);
		DrawEllipse(mainRect);
		SetStrokeStyle(p_prevStrokeStyle);
	}
	else {
		SetBrushColor(m_titleColor);
		DrawEllipse(mainRect);
	}

	// draw small circle
	const float SMALL_RADIUS = 7.0f;
	float offset = 2.0f;
	const DRect smallRect = {
		mainRect.right - SMALL_RADIUS - offset, mainRect.top + SMALL_RADIUS + offset,
		mainRect.right + SMALL_RADIUS - offset, mainRect.top - SMALL_RADIUS + offset,
	};
	FillEllipse(smallRect);

	// draw + on small circle
	offset = 3.0f;
	const float centerPosX = (smallRect.left + smallRect.right) / 2.0f;
	const float centerPosY = (smallRect.top + smallRect.bottom) / 2.0f;

	DPoint startPos = { smallRect.left + offset, centerPosY };
	DPoint endPos = { smallRect.right - offset, centerPosY };
	SetStrokeWidth(2.0f);
	SetBrushColor(m_textBackgroundColor);
	DrawLine(startPos, endPos);

	startPos = { centerPosX, smallRect.top - offset };
	endPos = { centerPosX, smallRect.bottom + offset };
	DrawLine(startPos, endPos);
	SetStrokeWidth(1.0f);
}

void ColorView::DrawHueCircle()
{
	for (const auto &hueData : m_hueDataList) {
		SetBrushColor(hueData.first);
		DrawLine(hueData.second.first, hueData.second.second);
	}
}

void ColorView::DrawLightnessCircle()
{
	if (nullptr == mp_lightnessGradientBrush) {
		FillEllipse(m_lightnessRect);

		return;
	}

	auto p_previousBrush = SetBrush(mp_lightnessGradientBrush);
	FillEllipse(m_lightnessRect);
	SetBrush(p_previousBrush);
}
