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
#include "Strings.h"

std::wstring ConvertToWide(const std::string& string)
{
	int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, string.c_str(), static_cast<int>(string.size()), nullptr, 0);
	std::wstring wide;
	wide.resize(count);
	count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, string.c_str(), static_cast<int>(string.size()), &wide[0], static_cast<int>(wide.size()));
	wide.resize(count);
	return wide;
}

std::string ConvertToMultiByte(const std::wstring& string)
{
	int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, string.c_str(), static_cast<int>(string.size()), nullptr, 0, nullptr, nullptr);
	std::string utf8;
	utf8.resize(count);
	count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, string.c_str(), static_cast<int>(string.size()), &utf8[0], static_cast<int>(utf8.size()), nullptr, nullptr);
	utf8.resize(count);
	return utf8;
}
