#ifndef _COLOR_MODEL_H_
#define _COLOR_MODEL_H_

const size_t INVALID_INDEX = static_cast<size_t>(-1);
const size_t INTERVAL = 40;
const size_t TEXT_HEIGHT = 35;
const size_t INDICATE_HEIGHT = 25;
const float DEFAULT_TRANSPARENCY = 0.7f;

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
	CBT hoverButton;
	CBT clickedButton;
}CMD;

#endif //!_COLOR_MODEL_H_