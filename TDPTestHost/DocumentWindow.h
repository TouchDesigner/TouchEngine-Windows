#pragma once

#include <string>
#include <atomic>
#include <map>
#include <memory>
#include <TDP/TouchPlugIn.h>
#include "Renderer.h"

class DocumentWindow
{
public:
    static HRESULT registerClass(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    DocumentWindow(std::wstring path);
    ~DocumentWindow();
    const std::wstring getPath() const;
    void openWindow(HWND parent);
    HWND getWindow() const { return myWindow; };
    void didLoad() { myDidLoad = true; };
    void parameterLayoutDidChange();
    void render();
    void cancelFrame();
private:
    static wchar_t *WindowClassName;
    static void eventCallback(TPInstance *instance, TPEvent event, TPResult result, int64_t time_value, int32_t time_scale, void * info);
    static void parameterValueCallback(TPInstance *instance, TPScope scope, int32_t group, int32_t index, void *info);
    static const double InputSampleRate;
    static const int32_t InputChannelCount;
    static const int64_t InputSampleLimit;
    static const int64_t InputSamplesPerFrame;
    void endFrame(int64_t time_value, int32_t time_scale, TPResult result);
    TPInstance *myInstance;
    HWND myWindow;
    std::unique_ptr<Renderer> myRenderer;
    std::atomic<bool> myDidLoad;
    std::atomic<bool> myInFrame;
    double myLastFloatValue;
    float myLastStreamValue;
    // TP group/index pair to renderer index
    std::map<std::pair<int32_t, int32_t>, size_t> myOutputParameterTextureMap;
};

