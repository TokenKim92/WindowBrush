#ifndef _PALETTE_MODEL_H_
#define _PALETTE_MODEL_H_

namespace PALETTE {

	const size_t INVALID_INDEX = static_cast<size_t>(-1);
	const float DEFAULT_TRANSPARENCY = 0.8f;

	const unsigned int DIALOG_WIDTH = 240;
	const unsigned int DIALOG_HEIGHT = static_cast<unsigned int>(DIALOG_WIDTH * 1.14);

	const long TITLE_HEIGHT = 35;

	const size_t INTERVAL = 40;
	const float COLOR_RADIUS = 10.0f;
	const float PLUS_BUTTON_RADIUS = 7.0f;
	
	// for PaletteAddView
	const float RETURN_ICON_X_MARGIN = 14.0f;
	const float RETURN_ICON_Y_MARGIN = 10.0f;
	
	const float LIGHTNESS_CIRCLE_RADIUS = DIALOG_WIDTH * 0.2f;
	const float HUE_CIRCLE_RADIUS = DIALOG_WIDTH * 0.33f;	
	const float HUE_STROKE_HALF_WIDTH = 2.5f;

	const float ADD_BUTTON_MARGIN = 20.0f;
	const float ADD_BUTTON_SIZE = 20.0f;

	const float INDICATE_MARGIN = 10.0f;
	const float INDICATE_WIDTH = 100.0f;
	const float INDICATE_HEIGHT = 35.0f;

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

#endif //!_PALETTE_MODEL_H_