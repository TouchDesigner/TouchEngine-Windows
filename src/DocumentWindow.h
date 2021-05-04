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

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <mutex>
#include <TouchEngine/TouchEngine.h>
#include "Renderer.h"

class DocumentWindow
{
public:
	enum class Mode {
		DirectX,
		OpenGL
	};
	static HRESULT registerClass(HINSTANCE hInstance);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	DocumentWindow(std::wstring path, Mode mode);
	~DocumentWindow();

	const std::wstring		getPath() const;
	void					openWindow(HWND parent);
	void					linkLayoutDidChange();
	void					update();
	void					render(bool loaded);

	Mode
	getMode() const
	{
		return myMode;
	}

	HWND
	getWindow() const
	{
		return myWindow;
	}
	void
	didLoad()
	{
		std::lock_guard<std::mutex> guard(myMutex);
		myDidLoad = true;
	}
private:
	static wchar_t* WindowClassName;
	static void		eventCallback(TEInstance * instance,
								TEEvent event,
								TEResult result,
								int64_t start_time_value,
								int32_t start_time_scale,
								int64_t end_time_value,
								int32_t end_time_scale,
								void * info);

	static void		linkEventCallback(TEInstance *instance, TELinkEvent event, const char *identifier, void *info);

	static const double		 InputSampleRate;
	static const int32_t	 InputChannelCount;
	static const int64_t	 InputSampleLimit;
	static const int64_t	 InputSamplesPerFrame;
	static const UINT_PTR	 UpdateTimerID;
	static constexpr int32_t FramesPerSecond{ 60 };
	static constexpr int32_t TimeRate{ 6000 };
	static constexpr UINT	 InitialWindowWidth{ 640 };
	static constexpr UINT	 InitialWindowHeight{ 480 };

	static constexpr unsigned int ImageWidth{ 256 };
	static constexpr unsigned int ImageHeight{ 256 };

	void	linkValueChange(const char* identifier);
	void	endFrame(int64_t time_value, int32_t time_scale, TEResult result);
	void	getState(bool& loaded, bool& linksChanged, bool& inFrame);
	void	setInFrame(bool inFrame);
	void	applyLayoutChange();
	bool	applyOutputTextureChange();
	int64_t	getRenderTime();

	std::wstring	myPath;
	Mode			myMode;
	TEInstance*		myInstance;
	HWND			myWindow;

	std::unique_ptr<Renderer>	myRenderer;

	bool			myDidLoad;
	bool			myInFrame;
	double			myLastFloatValue;
	TEResult		myLastResult{ TEResultSuccess };
	std::mutex		myMutex;
	LARGE_INTEGER	myStartTime{ 0 };
	LARGE_INTEGER	myPerformanceCounterFrequency{ 1 };

	// TE link identifier to renderer index
	std::map<std::string, size_t>	myOutputLinkTextureMap;
	std::vector<std::string>		myPendingOutputTextures;
	bool							myPendingLayoutChange;
};

