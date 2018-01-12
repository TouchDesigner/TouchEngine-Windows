#pragma once

#include <string>
#include <TouchPlugIn/TouchPlugIn.h>

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
private:
    static wchar_t *WindowClassName;
    TPRendererRef myRenderer;
    HWND myWindow;
};

