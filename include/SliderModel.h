#ifndef _SLIDER_MODEL_H_
#define _SLIDER_MODEL_H_

#include <string>

namespace SLIDER {

	const float DEFAULT_TRANSPARENCY = 0.8f;

	const unsigned int DIALOG_WIDTH = 350;
	const unsigned int DIALOG_HEIGHT = 220;

	const float TITLE_FONT_SIZE = 20.0f;
	const float TEXT_FONT_SIZE = 14.0f;

	const float TITLE_HEIGHT = 60.0f;

	const float SLIDER_X_MARGIN = 20.0f;
	const float SLIDER_Y_MARGIN = 35.0f;

	const float BUTTON_MARGIN = 10.0f;
	const float BUTTON_HEIGHT = 35.0f;

	const float THUMB_RADIUS = 10.0f;
	const float THUMB_MARGIN = 10.0f;
	const float THUMB_VALUE_WIDTH = 30.0f;
	const float THUMB_VALUE_HEIGHT = 40.0f;
	
	const float TIC_HALF_WIDTH = 1.0f;
	const float TIC_HALF_HEIGHT = 8.0f;

	typedef enum class BUTTON_TPYE
	{
		NONE,
		THUMB,
		SAVE,
		CANCEL
	}BT;

	typedef struct MODEL_DATA
	{
		BT hoverButtonType;
		BT clickedButtonType;
		size_t thumbIndex;
		DRect thumbRect;
	}MD;

	typedef struct RANGE_DATA
	{
		std::wstring minTitle;
		int min;
		std::wstring maxTitle;
		int max;
	}RD;

}

#endif //!_SLIDER_MODEL_H_