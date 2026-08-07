/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "Picture.h"
#include "Vec.h"
#include "Color.h"
#include <algorithm>

static float smoothstep(float edge0, float edge1, float x) {
	x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return x * x * (3.0f - 2.0f * x);
}

static float mix(float x, float y, float a)
{
	return x * (1.0f - a) + y * a;
}

TouchObject<TETexture> Picture::getCircleTexture(int width, int height, float radius, const Color& foreground, const Color& background, TextureProvider& provider)
{
    std::vector<unsigned char> tex(width * height * 4);

	Vec2 mid = { width / 2.0f, height / 2.0f };

	float edge = min(mid.values[0], mid.values[1]) * radius;

	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			float offset = mid.distance({float(x), float(y)});
			float amount = smoothstep(edge, edge - 3, offset);

			tex[(y * width * 4) + (x * 4) + 0] = static_cast<unsigned char>(mix(background.blue, foreground.blue, amount) * 255);
			tex[(y * width * 4) + (x * 4) + 1] = static_cast<unsigned char>(mix(background.green, foreground.green, amount) * 255);
			tex[(y * width * 4) + (x * 4) + 2] = static_cast<unsigned char>(mix(background.red, foreground.red, amount) * 255);
			tex[(y * width * 4) + (x * 4) + 3] = static_cast<unsigned char>(mix(background.alpha, foreground.alpha, amount) * 255);
		}
	}

	return provider.getTexture(tex.data(), width * 4 * sizeof(char), width, height);
}
