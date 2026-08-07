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
#include <cmath>
#include <array>

template <size_t N>
struct Vec {
	float values[N];

	float distance(const Vec<N>& other) const {
		float sum = 0.0f;
		for (int i = 0; i < N; ++i) {
			float diff = values[i] - other.values[i];
			sum += diff * diff;
		}
		return std::sqrt(sum);
	}
};

using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;
