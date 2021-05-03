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
#include "FileReader.h"


FileReader::FileReader(const std::wstring &path)
	: myFile(INVALID_HANDLE_VALUE)
{
	CREATEFILE2_EXTENDED_PARAMETERS extended = { 0 };
	extended.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
	extended.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	extended.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
	extended.dwSecurityQosFlags = SECURITY_ANONYMOUS;
	extended.lpSecurityAttributes = nullptr;
	extended.hTemplateFile = nullptr;

	myFile = CreateFile2(path.data(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extended);
}


FileReader::~FileReader()
{
	if (myFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(myFile);
	}
}

FileReader::FileReader(FileReader && o)
	: myFile(o.myFile)
{
	o.myFile = INVALID_HANDLE_VALUE;
}

FileReader&
FileReader::operator=(FileReader && o)
{
	if (&o != this)
	{
		if (myFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(myFile);
		}
		myFile = o.myFile;
		o.myFile = INVALID_HANDLE_VALUE;
	}
	return *this;
}

bool
FileReader::read(std::vector<unsigned char>& destination)
{
	FILE_STANDARD_INFO info = { 0 };
	if (!GetFileInformationByHandleEx(myFile, FileStandardInfo, &info, sizeof(info)))
	{
		return false;
	}
	if (info.EndOfFile.HighPart != 0)
	{
		return false;
	}
	destination.resize(info.EndOfFile.LowPart);
	if (!ReadFile(myFile, destination.data(), static_cast<DWORD>(destination.size()), nullptr, nullptr))
	{
		return false;
	}
	return true;
}
