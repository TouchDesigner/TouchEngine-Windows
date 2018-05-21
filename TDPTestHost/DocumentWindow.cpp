#include "stdafx.h"
#include "DocumentWindow.h"
#include "DocumentManager.h"
#include "Resource.h"
#include <codecvt>
#include <array>

wchar_t *DocumentWindow::WindowClassName = L"DocumentWindow";

HINSTANCE theInstance;

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
    wndClass.lpszClassName = WindowClassName;
    wndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wndClass))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
        {
            return HRESULT_FROM_WIN32(dwError);
        }
    }

    theInstance = hInstance;

    return S_OK;
}

LRESULT CALLBACK DocumentWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CLOSE:
    {
        HMENU menu = GetMenu(hWnd);
        if (menu)
        {
            DestroyMenu(menu);
        }
        DestroyWindow(hWnd);
        break;
    }
    case WM_DESTROY:
    {
        auto document = DocumentManager::sharedManager().lookup(hWnd);
        if (document)
        {
            document->myWindow = nullptr;
        }
        DocumentManager::sharedManager().didClose(hWnd);
        break;
    }
    case WM_LBUTTONUP:
    {
        auto document = DocumentManager::sharedManager().lookup(hWnd);
        if (document)
        {
            TPRendererPingaling(document->myRenderer);
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return LRESULT();
}

void DocumentWindow::frameCallback(double time, TPError error, void * TP_NULLABLE info /* TODO: etc. */)
{
    DocumentWindow *window = static_cast<DocumentWindow *>(info);
    window->endFrame(time, error);
}

void DocumentWindow::endFrame(double time, TPError error)
{
    if (error != TPErrorNone)
    {
        // TODO: pass it out
    }
    else
    {
        std::array<float, 300> channel1;
        std::array<float, 300> channel2;
        std::array<float *, 2> channels{ channel1.data(), channel2.data() };
        uint64_t length = 300;
        TPRendererPropertyGetStreamValues(myRenderer, TPScopeOutput, 0, channels.data(), 2, &length);
        if (channel1 != channel2)
        {
            int i = 2;
        }
        
        for (int i = 0; i < length; i++)
        {
            float value = channel1[i];
            if (myLastStreamValue != value - 1.0)
            {
                // This will "fail" when it reaches the first integer that can't be represented by a float, that's ok
                int j = 3;
            }
            myLastStreamValue = value;
        }
    }
}

static void myPropertyStateCallback(void *info)
{

}

static void myPropertyValueCallback(TPScope scope, unsigned int index, void *info)
{

}

DocumentWindow::DocumentWindow(std::wstring path)
    : myRenderer(nullptr), myWindow(nullptr), myLastStreamValue(-1.0f)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string utf8 = converter.to_bytes(path);
    myRenderer = TPRendererCreate(utf8.c_str(), TPTimeExternal, frameCallback, myPropertyStateCallback, myPropertyValueCallback, this);
}


DocumentWindow::~DocumentWindow()
{
    if (myWindow)
    {
        PostMessageW(myWindow, WM_CLOSE, 0, 0);
    }
    TPRendererDestroy(myRenderer);
}

const std::wstring DocumentWindow::getPath() const
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(TPRendererGetPath(myRenderer));
}

void DocumentWindow::openWindow(HWND parent)
{
    RECT rc;
    SetRect(&rc, 0, 0, 640, 480);
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

    std::wstring path = getPath();
    myWindow = CreateWindowW(WindowClassName,
        path.data(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        (rc.right - rc.left), (rc.bottom - rc.top),
        parent,
        nullptr,
        theInstance,
        0);

    HRESULT result;
    if (myWindow)
    {
        ShowWindow(myWindow, SW_SHOW);
        UpdateWindow(myWindow);
        result = S_OK;
    }
    else
    {
        DWORD error = GetLastError();
        result = HRESULT_FROM_WIN32(error);
    }
    if (SUCCEEDED(result))
    {
        myDevice.createDeviceResources();
        myDevice.createWindowResources(myWindow);
    }
}

void DocumentWindow::render()
{
    TPRendererStartFrame(myRenderer);
}
