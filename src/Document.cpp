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
#include <filesystem>
#include "Document.h"
#include "Application.h"
#include "OpenGLRenderer.h"
#include "DX11Renderer.h"
#include "DX12Renderer.h"
#include "Strings.h"
#include "Geometry.h"
#include "Picture.h"
#include "EaseInAnimation.h"

using ProgressAnimation = EaseInAnimation<4.0>;

static std::unique_ptr<Renderer> getRenderer(Graphics mode, const Window &window)
{
	std::unique_ptr<Renderer> renderer;
	switch (mode)
	{
	case Graphics::OpenGL:
		renderer = std::make_unique<OpenGLRenderer>();
		break;
	case Graphics::DX11:
		renderer = std::make_unique<DX11Renderer>();
		break;
	case Graphics::DX12:
		renderer = std::make_unique<DX12Renderer>();
		break;
	default:
		throw std::runtime_error("Unexpected graphics mode");
		break;
	}
	renderer->setup(window.getHWND());
	return renderer;
}

static DWORD getWindowStyleFlags(Graphics mode)
{
	switch (mode)
	{
	case Graphics::OpenGL:
		return CS_OWNDC | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	case Graphics::DX11:
	case Graphics::DX12:
	default:
		return 0;
	}
}

Document::Document(Application& application, Graphics mode)
	: myMode(mode),
	myApplication(application),
	myWindow(application.getInstance(), getWindowStyleFlags(mode), FrameRate, *this),
	myRenderer(getRenderer(mode, myWindow)),
	myEngine(myRenderer->getTEContext(), *this)
{
	myRenderer->setBackgroundColor(Palette::LightGray);
}

Document::~Document()
{
	// end the instance so associated resources are released
	// before we destroy the renderer
	myEngine.end();
	if (myRenderer)
	{
		myRenderer->stop();
	}
}

void Document::open(const std::string& path)
{
	myEngine.open(path);
	
	std::filesystem::path name = std::filesystem::path(path).filename();

	std::string title = name.string();
	switch (myMode)
	{
	case Graphics::OpenGL:
		title += " (OpenGL - ";
		break;
	case Graphics::DX11:
		title += " (DirectX 11 - ";
		break;
	case Graphics::DX12:
		title += " (DirectX 12 - ";
		break;
	default:
		break;
	}
	title += myRenderer->getDeviceName();
	title += ")";
	myWindow.setTitle(title);
	myWindow.open();
}

void Document::windowUpdate()
{
	myEngine.update();

	if (myEngine.isLoaded() && !myEngine.isInFrame())
	{
		Rational now = getTime();

		if (myProgressUpdate)
		{
			static constexpr const char* kProgressInputName = "Progress";
			std::string progressInputID = myEngine.doubleInput(kProgressInputName);
			if (!progressInputID.empty())
			{
				myEngine.setInput(progressInputID, ProgressAnimation::progress(now));
			}

			if (ProgressAnimation::isFinished(now))
			{
				myProgressUpdate = false;
			}
		}
		if (myInputUpdateGeometry)
		{
			myInputUpdateGeometry = false;
			std::string firstGeometryInputID = myEngine.firstGeometryInput();
			if (!firstGeometryInputID.empty())
			{
				static constexpr int kNumDivisions = 16;
				TouchObject<TEGeometry> geometry = Geometry::getCircleGeometry(1.0, kNumDivisions, Palette::LightPink, Palette::BrightGray, *myRenderer);
				if (!geometry)
				{
					throw std::runtime_error("Couldn't generate input geometry");
				}
				myEngine.setInput(firstGeometryInputID, geometry);
			}
		}
		if (myInputUpdateTexture)
		{
			myInputUpdateTexture = false;
			std::string firstTextureInputID = myEngine.firstTextureInput();
			if (!firstTextureInputID.empty())
			{
				TouchObject<TETexture> texture = Picture::getCircleTexture(myWindow.width(), myWindow.height(), 0.4f, Palette::LightYellow, Palette::Clear, *myRenderer);

				if (!texture)
				{
					throw std::runtime_error("Couldn't generate input texture");
				}
				myEngine.setInput(firstTextureInputID, texture, myRenderer->getTEContext());
			}
		}

		myRenderer->addResourceTransfers(myEngine.getInstance());

		myEngine.nextFrame(now);
	}

	std::string firstTextureOutputID = myEngine.firstTextureOutput();
	if (!firstTextureOutputID.empty() && myEngine.outputHasChanged(firstTextureOutputID))
	{
		TouchObject<TETexture> texture;
		myEngine.getOutput(firstTextureOutputID, texture);

		TouchObject<TESemaphore> semaphore;
		uint64_t waitValue;
		if (myEngine.getResourceTransfer(texture, semaphore, waitValue))
		{
			myRenderer->setOutputImage(texture, semaphore, waitValue);
		}
		else
		{
			myRenderer->setOutputImage(texture);
		}
	}
	myRenderer->render();
}

void Document::windowClose()
{
	myApplication.documentWindowDidClose();
}

void Document::windowResize(unsigned int w, unsigned int h)
{
	myRenderer->resize(w, h);
	// Resize our texture input to match window size
	myInputUpdateTexture = true;
}

const Window& Document::getParentWindow() const
{
	return myApplication.getWindow();
}

void Document::engineConfigured()
{
	std::string error;
	if (!myRenderer->configure(myEngine.getInstance(), error))
	{
		throw std::runtime_error(error);
	}
}

void Document::engineLoaded()
{
	// Set inputs after load
	myInputUpdateGeometry = myInputUpdateTexture = true;
	myProgressUpdate = true;
	myRenderer->setBackgroundColor(Palette::LightBlue);
}

Rational Document::getTime()
{
	auto now = std::chrono::steady_clock::now();
	if (myStartTime == std::chrono::steady_clock::time_point())
	{
		myStartTime = now;
	}
	auto diff = now - myStartTime;
	return Rational{
		.numerator = diff.count() * std::chrono::steady_clock::period::num,
		.denominator = std::chrono::steady_clock::period::den
	};
}
