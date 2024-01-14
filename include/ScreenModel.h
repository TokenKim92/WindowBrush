#ifndef _SCREEN_MODEL_H_
#define _SCREEN_MODEL_H_

namespace SCREEN {

	const float DEFAULT_TRANSPARENCY = 0.8f;

	const unsigned int DIALOG_WIDTH = 350;
	const unsigned int DIALOG_HEIGHT = 220;

	const float TITLE_FONT_SIZE = 20.0f;
	const float TEXT_FONT_SIZE = 14.0f;

	const float TITLE_HEIGHT = 60.0f;

	const float BUTTON_MARGIN = 10.0f;
	const float BUTTON_HEIGHT = 35.0f;

	typedef enum class BUTTON_TPYE
	{
		NONE,
		SAVE,
		CANCEL
	}BT;

	typedef struct MODEL_DATA
	{
		BT hoverButtonType;
		BT clickedButtonType;
	}MD;
}

#endif //!_SCREEN_MODEL_H_