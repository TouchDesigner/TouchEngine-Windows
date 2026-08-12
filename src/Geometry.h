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

#include <TouchEngine/TouchEngine.h>
#include <vector>

struct Color;

namespace Geometry {

class BufferProvider {
public:
	virtual TouchObject<TEBuffer>	getDeviceBuffer(const void* src, size_t size);
	virtual TouchObject<TEBuffer>	getHostBuffer(const void* src, size_t size);

	template <class T>
	TouchObject<TEBuffer>			getDeviceBuffer(const std::vector<T>& vector)
	{
		return getDeviceBuffer(vector.data(), sizeof(T) * vector.size());
	}
	template <class T>
	TouchObject<TEBuffer>			getHostBuffer(const std::vector<T>& vector)
	{
		return getHostBuffer(vector.data(), sizeof(T) * vector.size());
	}
};

TouchObject<TEGeometry> getCircleGeometry(float radius, int divisions, const Color &color1, const Color &color2, BufferProvider &provider);

}
