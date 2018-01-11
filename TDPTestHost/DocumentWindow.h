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
    const std::wstring& getPath() const;
private:
    TPRendererRef myRenderer;
};

