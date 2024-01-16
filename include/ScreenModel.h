#ifndef _SCREEN_MODEL_H_
#define _SCREEN_MODEL_H_

namespace SCREEN {

	const size_t INVALID_INDEX = static_cast<size_t>(-1);

	const float DEFAULT_TRANSPARENCY = 0.8f;

	const unsigned int DIALOG_WIDTH = 350;
	const unsigned int DIALOG_HEIGHT = 220;

	const float TITLE_FONT_SIZE = 20.0f;
	const float TEXT_FONT_SIZE = 14.0f;

	const float TITLE_HEIGHT = 60.0f;

	const float SCREEN_X_MARGIN = 30.0f;
	const float SCREEN_Y_MARGIN = 20.0f;

	const float BUTTON_MARGIN = 20.0f;
	const float BUTTON_HEIGHT = 35.0f;

	typedef enum class BUTTON_TPYE
	{
		NONE,
		SAVE,
		CANCEL,
		SCREEN
	}BT;

	typedef struct MODEL_DATA
	{
		BT hoverButtonType;
		BT clickedButtonType;
		size_t hoverScreenIndex;
		size_t clickedScreenIndex;
	}MD;
}

#endif //!_SCREEN_MODEL_H_