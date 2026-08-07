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
#include "DocumentWindow.h"

void DocumentWindow::resize(unsigned int w, unsigned int h)
{
	Window::resize(w, h);
	myObserver.windowResize(w, h);
}

void DocumentWindow::timer(UINT_PTR timerID)
{
	if (timerID == UpdateTimerID)
	{
		myObserver.windowUpdate();
	}
}

void DocumentWindow::destroy()
{
	myObserver.windowClose();
}

DocumentWindow::Config DocumentWindow::DocumentWindowConfig = {
	(const LPWSTR)L"DocumentWindow",
	(const LPWSTR)nullptr,
	Window::Config::Background::BlackBrush,
	InitialWindowWidth,
	InitialWindowHeight
};

DocumentWindow::DocumentWindow(HINSTANCE h, DWORD styleFlags, const Rational &frameRate, Observer &observer)
	: Window(DocumentWindowConfig, h, (LPWSTR)L"", observer.getParentWindow(), styleFlags),
	myObserver(observer),
	myFrameRate(frameRate)
{
	
}

void DocumentWindow::open(int nCmdShow)
{
	Window::open(nCmdShow);
	const auto interval = static_cast<UINT>(std::ceil(1000. * myFrameRate.numerator / myFrameRate.denominator));
	setTimer(UpdateTimerID, interval);
}
