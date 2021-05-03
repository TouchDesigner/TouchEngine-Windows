/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchEngine
 *
 * Copyright © 2021 Derivative. All rights reserved.
 *
 */

#pragma once

#include <string>
#include <atomic>
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
    const std::wstring getPath() const;
    Mode getMode() const { return myMode; }
    void openWindow(HWND parent);
    HWND getWindow() const { return myWindow; };
    void didLoad() { myDidLoad = true; };
    void parameterLayoutDidChange();
    void render();
    void cancelFrame();
private:
    static wchar_t *WindowClassName;
    static void eventCallback(TEInstance * instance,
								TEEvent event,
								TEResult result,
								int64_t start_time_value,
								int32_t start_time_scale,
								int64_t end_time_value,
								int32_t end_time_scale,
								void * info);
    static void parameterEventCallback(TEInstance *instance, TELinkEvent event, const char *identifier, void *info);
    static const double InputSampleRate;
    static const int32_t InputChannelCount;
    static const int64_t InputSampleLimit;
    static const int64_t InputSamplesPerFrame;
    static const UINT_PTR RenderTimerID;
	static constexpr int32_t TimeRate{ 6000 };
    static constexpr UINT InitialWindowWidth{ 640 };
    static constexpr UINT InitialWindowHeight{ 480 };
    static constexpr unsigned int ImageWidth{ 256 };
    static constexpr unsigned int ImageHeight{ 256 };
    void parameterValueChange(const char* identifier);
    void endFrame(int64_t time_value, int32_t time_scale, TEResult result);
    void applyLayoutChange();
    void applyOutputTextureChange();
	std::wstring myPath;
    Mode myMode;
    TEInstance *myInstance;
    HWND myWindow;
    std::unique_ptr<Renderer> myRenderer;
    std::atomic<bool> myDidLoad;
    std::atomic<bool> myInFrame;
    double myLastFloatValue;
    // TE param identifier to renderer index
    std::map<std::string, size_t> myOutputParameterTextureMap;
    std::mutex myMutex;
    std::vector<std::string> myPendingOutputTextures;
    std::atomic<bool> myPendingLayoutChange;
	int64_t myTime{ 0 };
};

