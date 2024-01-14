#ifndef _COLOR_MODEL_H_
#define _COLOR_MODEL_H_

namespace COLOR {

	const size_t INVALID_INDEX = static_cast<size_t>(-1);
	const float DEFAULT_TRANSPARENCY = 0.8f;

	const unsigned int DIALOG_WIDTH = 240;
	const unsigned int DIALOG_HEIGHT = static_cast<unsigned int>(DIALOG_WIDTH * 1.14);

	const size_t INTERVAL = 40;
	const size_t TITLE_HEIGHT = 35;
	const size_t INDICATE_HEIGHT = 25;

	const float LIHTNESS_CIRCLE_RADIUS = DIALOG_WIDTH * 0.2f;
	const float HUE_CIRCLE_RADIUS = DIALOG_WIDTH * 0.33f;
	const float BUTTON_RADIUS = 10.0f;

	typedef enum class DRAW_MODE
	{
		SELECT,
		ADD
	}DM;

	typedef enum class BUTTON_TYPE
	{
		NONE,
		RETURN,
		HUE,
		LIGHTNESS,
		ADD
	}BT;

	typedef struct MODEL_DATA
	{
		size_t hoverIndex;
		size_t clickedIndex;
		BT hoverButtonType;
		BT clickedButtonType;
		DRect hueButtonRect;
		DRect lightnessButtonRect;
	}MD;

}

#endif //!_COLOR_MODEL_H_