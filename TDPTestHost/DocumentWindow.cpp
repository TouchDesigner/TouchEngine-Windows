#include "stdafx.h"
#include "DocumentWindow.h"
#include "Resource.h"
#include <codecvt>

HRESULT DocumentWindow::registerClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wndClass;
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = DocumentWindow::WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TDPTESTHOST));
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = L"DocumentWindow";
    wndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wndClass))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
        {
            return HRESULT_FROM_WIN32(dwError);
        }
    }
}

LRESULT DocumentWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return LRESULT();
}

DocumentWindow::DocumentWindow(std::wstring path)
    : myRenderer(nullptr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string utf8 = converter.to_bytes(path);
    myRenderer = TPRendererCreate(utf8.c_str(), TPRendererTimeExternal, nullptr, nullptr);
}


DocumentWindow::~DocumentWindow()
{
}

const std::wstring & DocumentWindow::getPath() const
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(TPRendererGetPath(myRenderer));
}
