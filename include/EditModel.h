#ifndef _EDIT_MODEL_H_
#define _EDIT_MODEL_H_

#include "ColorPalette.h"
#include <vector>
#include <string>

namespace EDIT {
	const size_t INVALID_INDEX = static_cast<size_t>(-1);

	const unsigned int DIALOG_WIDTH = 300;
	const unsigned int DIALOG_HEIGHT = 230;

	const float DEFAULT_TRANSPARENCY = 0.8f;

	const float TITLE_FONT_SIZE = 20.0f;
	const float TEXT_FONT_SIZE = 14.0f;

	const float TITLE_HEIGHT = 60.0f;

	const float EDIT_MARGIN = 20.0f;
	const float EDIT_WIDTH = 150;
	const float EDIT_HEIGHT = 35.0;
	const float EDIT_TITLE_HEIGHT = 22.0f;

	const float WARNING_HEIGHT = 35.0;

	const float BUTTON_MARGIN = 10.0f;
	const float BUTTON_HEIGHT = 35.0f;

	const float MAX_DIGIT_LEGNHT = 3;

	const DColor WARNING_COLOR = RGB_TO_COLORF(RED_300);
	const std::wstring WARNING_TEXT = L"(!) Invalid number value.";

	const float BUTTON_ROUND_RADIUS = 5.0f;

	typedef enum class BUTTON_TYPE
	{
		NONE,
		EDIT,
		SAVE,
		CANCEL
	}BT;

	typedef struct MODEL_DATA
	{
		BT hoverButtonType;
		BT clickedButtonType;
		size_t hoverEditIndex;
		size_t clickedEditIndex;

		std::vector<unsigned int> valueList;
	}MD;

	struct RANGE
	{
		unsigned int min;
		unsigned int max;
	};
}

#endif //!_EDIT_MODEL_H_