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
    static void frameCallback(double time, TPError error, void * TP_NULLABLE info);
    void endFrame(double time, TPError error);
    TPInstance *myInstance;
    HWND myWindow;
    Device myDevice;
    float myLastStreamValue;
};

