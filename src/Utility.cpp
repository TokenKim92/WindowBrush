#include <d2d1.h>
#include "Utility.h"

D2D1_COLOR_F fromHueToColorF(const float hue)
{
	const size_t colorCount = 3;
	const float saturation = 1.0f;
	const float lightness = 0.5;
	float rgb[colorCount];

	const float q = lightness + saturation - lightness * saturation;
	const float p = 2.0f * lightness - q;
	float t[colorCount] = { hue + 2.0f, hue, hue - 2.0f };

	for (int i = 0; i < colorCount; ++i) {
		if (t[i] < 0.0f) {
			t[i] += 6.0f;
		}
		else if (t[i] > 6.0f) {
			t[i] -= 6.0f;
		}

		if (t[i] < 1.0f) {
			rgb[i] = p + (q - p) * t[i];
		}
		else if (t[i] < 3.0f) {
			rgb[i] = q;
		}
		else if (t[i] < 4.0f) {
			rgb[i] = p + (q - p) * (4.0f - t[i]);
		}
		else {
			rgb[i] = p;
		}
	}

	return D2D1_COLOR_F({ rgb[0], rgb[1], rgb[2], 1.0f });
}
