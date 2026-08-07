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

#include "Window.h"
#include "Rational.h"

class DocumentWindow : public Window
{
public:
	class Observer {
	public:
		virtual void windowUpdate() = 0;
		virtual void windowClose() = 0;
		virtual void windowResize(unsigned int w, unsigned int h) = 0;
		virtual const Window& getParentWindow() const = 0;
	};
	DocumentWindow(HINSTANCE h, DWORD styleFlags, const Rational &frameRate, Observer &observer);

	virtual void open(int nCmdShow = SW_SHOW) override;

protected:
	virtual void resize(unsigned int w, unsigned int h) override;
	virtual void timer(UINT_PTR timerID) override;
	virtual void destroy() override;
private:
	static Config DocumentWindowConfig;

	static constexpr UINT_PTR	UpdateTimerID = 1;

	static constexpr UINT	 InitialWindowWidth{ 800 };
	static constexpr UINT	 InitialWindowHeight{ 600 };

	Observer& myObserver;
	Rational myFrameRate;
};

