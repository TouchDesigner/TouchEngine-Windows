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

#pragma once
#include <stdexcept>

struct Color
{
	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	float alpha = 1.0f;
	float operator[](int i) const
	{
		switch (i)
		{
		case 0:
			return red;
		case 1:
			return green;
		case 2:
			return blue;
		case 3:
			return alpha;
		default:
			throw std::logic_error("index out of bounds");
			break;
		}
		return 0.0;
	}
};

struct Palette {
	static constexpr Color LightPink = { 1.0f, 0.8f, 0.8f, 1.0f };
	static constexpr Color LightGray = { 0.8f, 0.8f, 0.8f, 1.0f };
	static constexpr Color BrightGray = { 0.9f, 0.9f, 0.9f, 1.0f };
	static constexpr Color LightYellow = { 0.96f, 0.93f, 0.66f, 1.0f };
	static constexpr Color LightBlue = { 0.8f, 0.8f, 0.9f, 1.0f };
	static constexpr Color Clear = { 0.0f, 0.0f, 0.0f, 0.0f };
};
