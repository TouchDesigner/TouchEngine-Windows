#pragma once

#include <string>
#include <TouchPlugIn/TouchPlugIn.h>
#include "Device.h"

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
    void render();
private:
    static wchar_t *WindowClassName;
    static void eventCallback(TPInstance *instance, TPEvent event, TPError error, double time, void * info);
    static void propertyValueCallback(TPInstance *instance, TPScope scope, int32_t index, void *info);
    void endFrame(double time, TPError error);
    TPInstance *myInstance;
    HWND myWindow;
    Device myDevice;
    float myLastStreamValue;
};

