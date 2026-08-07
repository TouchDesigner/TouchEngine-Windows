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
#include <memory>
#include <string>
#include <chrono>
#include "DocumentWindow.h"
#include "Engine.h"
#include "Graphics.h"
#include "Rational.h"

class Renderer;
class Application;

class Document : public DocumentWindow::Observer, public Engine::Observer
{
public:
	Document(Application &application, Graphics mode);
	~Document();
	void open(const std::string& path);
private:
	// Inherited via DocumentWindow::Observer
	virtual void windowUpdate() override;
	virtual void windowClose() override;
	virtual void windowResize(unsigned int w, unsigned int h) override;
	virtual const Window& getParentWindow() const override;

	// Inherited via Engine::Observer
	virtual void engineConfigured() override;
	virtual void engineLoaded() override;

	Rational		getTime();

	static constexpr Rational	FrameRate = { .numerator = 1, .denominator = 60 };
	Graphics					myMode;
	Application&				myApplication;
	DocumentWindow				myWindow;
	std::unique_ptr<Renderer>	myRenderer;
	Engine						myEngine;
	bool						myInputUpdateGeometry = false;
	bool						myInputUpdateTexture = false;
	bool						myProgressUpdate = false;
	std::chrono::steady_clock::time_point myStartTime;
};
