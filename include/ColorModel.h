#ifndef _COLOR_MODEL_H_
#define _COLOR_MODEL_H_

const size_t INVALID_INDEX = static_cast<size_t>(-1);

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
	BT hoverButton;
	BT clickedButton;
}MD;

#endif //!_COLOR_MODEL_H_