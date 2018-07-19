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
            // document->cancelFrame();
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return LRESULT();
}

void DocumentWindow::eventCallback(TPInstance * instance, TPEvent event, TPError error, int64_t time_value, int32_t time_scale, void * info)
{
    DocumentWindow *window = static_cast<DocumentWindow *>(info);

    switch (event)
    {
    case TPEventInstanceDidLoad:
        window->didLoad();
        break;
    case TPEventInstanceParameterLayoutDidChange:
        window->parameterLayoutDidChange();
        break;
    case TPEventFrameDidFinish:
        window->endFrame(time_value, time_scale, error);
        break;
    default:
        break;
    }
}

void DocumentWindow::propertyValueCallback(TPInstance * instance, TPScope scope, int32_t group, int32_t index, void * info)
{

}

void DocumentWindow::endFrame(int64_t time_value, int32_t time_scale, TPError error)
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
        int64_t length = 300;
        error = TPInstanceParameterGetOutputStreamValues(myInstance, 0, 1, channels.data(), 2, &length);
        if (error == TPErrorNone)
        {
            // We would use the channel data here
        }
    }
    myInFrame = false;
}

DocumentWindow::DocumentWindow(std::wstring path)
    : myInstance(nullptr), myWindow(nullptr), myDidLoad(false), myInFrame(false), myLastStreamValue(-1.0f)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string utf8 = converter.to_bytes(path);
    myInstance = TPInstanceCreate(utf8.c_str(), TPTimeExternal, eventCallback, propertyValueCallback, this);
}


DocumentWindow::~DocumentWindow()
{
    if (myWindow)
    {
        PostMessageW(myWindow, WM_CLOSE, 0, 0);
    }
    if (myInstance)
    {
        TPInstanceDestroy(myInstance);
    }
}

const std::wstring DocumentWindow::getPath() const
{
    if (myInstance == nullptr)
    {
        return std::wstring();
    }
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(TPInstanceGetPath(myInstance));
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

void DocumentWindow::parameterLayoutDidChange()
{
    int32_t groups;
    TPError result = TPInstanceGetParameterGroupCount(myInstance, TPScopeInput, &groups);
    if (result == TPErrorNone)
    {
        for (int32_t i = 0; i < groups; i++)
        {
            int32_t properties;
            result = TPInstanceGetParameterCount(myInstance, TPScopeInput, i, &properties);
            if (result == TPErrorNone)
            {
                for (int32_t j = 0; j < properties; j++)
                {
                    TPParameterType type;
                    TPError result = TPInstanceParameterGetType(myInstance, TPScopeInput, i, j, &type);
                    if (result == TPErrorNone && type == TPParameterTypeFloatStream)
                    {
                        result = TPInstanceParameterSetInputStreamDescription(myInstance, i, j, 400.0, 1, 4000);
                    }
                }
            }
        }
    }
}

void DocumentWindow::render()
{
    if (myDidLoad && !myInFrame)
    {
        myInFrame = true;
        TPInstanceStartFrameAtTime(myInstance, 1000, 1000 * 1000);
    }
}

void DocumentWindow::cancelFrame()
{
    TPError result = TPInstanceCancelFrame(myInstance);
    if (result != TPErrorNone)
    {
        // TODO: post it
    }
}
