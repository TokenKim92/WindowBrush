#ifndef _COLOR_MODEL_H_
#define _COLOR_MODEL_H_

const size_t INVALID_INDEX = static_cast<size_t>(-1);
const unsigned int COLOR_DIALOG_WIDTH = 240;
const unsigned int COLOR_DIALOG_HEGIHT = 275;
const SIZE COLOR_DILAOG_SIZE = { 240, 275 };
const size_t INTERVAL = 40;
const size_t TITLE_HEIGHT = 35;
const size_t INDICATE_HEIGHT = 25;
const float DEFAULT_TRANSPARENCY = 0.8f;
const float LIHTNESS_CIRCLE_RADIUS = COLOR_DILAOG_SIZE.cx * 0.2f;
const float HUE_CIRCLE_RADIUS = COLOR_DILAOG_SIZE.cx * 0.33f;
const float HUE_BUTTON_RADIUS = 10.0f;

typedef enum class COLOR_DRAW_MODE
{
	SELECT,
	ADD
}CDM;

typedef enum class COLOR_BUTTON_TYPE
{
	NONE,
	RETURN,
	HUE,
	LIGHTNESS,
	ADD
}CBT;

typedef struct COLOR_MODEL_DATA
{
	size_t hoverIndex;
	size_t clickedIndex;
	CBT hoverButtonType;
	CBT clickedButtonType;
	DRect hueButtonRect;
}CMD;

#endif //!_COLOR_MODEL_H_