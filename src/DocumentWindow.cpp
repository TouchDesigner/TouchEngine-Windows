#include "stdafx.h"
#include <commdlg.h>
#include "DocumentWindow.h"
#include "Resource.h"
#include "DirectXRenderer.h"
#include "OpenGLRenderer.h"
#include <codecvt>
#include <array>

wchar_t *DocumentWindow::WindowClassName = L"DocumentWindow";
const int32_t DocumentWindow::InputChannelCount = 2;
const double DocumentWindow::InputSampleRate = 44100.0;
const int64_t DocumentWindow::InputSampleLimit = 44100 / 2;
const int64_t DocumentWindow::InputSamplesPerFrame = 44100 / 60;
const UINT_PTR DocumentWindow::RenderTimerID = 1;

static std::shared_ptr<DocumentWindow> theOpenDocument;

#define MAX_LOADSTRING 100

HINSTANCE theInstance;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
std::shared_ptr<DocumentWindow>   Open(HWND, DocumentWindow::Mode mode);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_TETESTHOST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	DocumentWindow::registerClass(hInst);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TETESTHOST));

	MSG msg;

	// Main message loop:
	msg.message = WM_NULL;

	while (msg.message != WM_QUIT)
	{
		bool got = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0;
		if (got)
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TETESTHOST));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TETESTHOST);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	RECT rc;
	SetRect(&rc, 0, 0, 600, 320);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		(rc.right - rc.left), (rc.bottom - rc.top), nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FILE_OPEN:
			theOpenDocument = Open(hWnd, DocumentWindow::Mode::DirectX);
			break;
		case ID_FILE_OPENOPENGL:
			theOpenDocument = Open(hWnd, DocumentWindow::Mode::OpenGL);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

std::shared_ptr<DocumentWindow> 
Open(HWND hWnd, DocumentWindow::Mode mode)
{
	WCHAR buffer[MAX_PATH + 1] = { 0 };
	OPENFILENAME ofns = { 0 };
	ofns.lStructSize = sizeof(OPENFILENAME);
	ofns.lpstrFile = buffer;
	ofns.nMaxFile = MAX_PATH;
	ofns.lpstrTitle = L"Select a file to open plz";
	BOOL result = GetOpenFileName(&ofns);
	if (result)
	{
		std::shared_ptr<DocumentWindow> win(std::make_shared<DocumentWindow>(buffer, mode));
		win->openWindow(hWnd);

		return win;
	}
	return std::shared_ptr<DocumentWindow>();
}


HRESULT DocumentWindow::registerClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wndClass;
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = DocumentWindow::WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TETESTHOST));
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
        KillTimer(hWnd, RenderTimerID);
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
		if (theOpenDocument && theOpenDocument->getWindow() == hWnd)
		{
			theOpenDocument = nullptr;
		}
        break;
    }
    case WM_LBUTTONUP:
    {
		if (theOpenDocument && theOpenDocument->getWindow() == hWnd)
        {
            // document->cancelFrame();
        }
        break;
    }
	case WM_SIZE:
	{
		if (theOpenDocument && theOpenDocument->getWindow() == hWnd)
		{
			theOpenDocument->myRenderer->resize(0, 0);
		}
		break;
	}
    case WM_TIMER:
    {
        if (wParam == RenderTimerID)
        {
            if (theOpenDocument)
            {
                theOpenDocument->render();
            }
        }

        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return LRESULT();
}

void DocumentWindow::eventCallback(TEInstance * instance,
									TEEvent event,
									TEResult result,
									int64_t start_time_value,
									int32_t start_time_scale,
									int64_t end_time_value,
									int32_t end_time_scale,
									void * info)
{
    DocumentWindow *window = static_cast<DocumentWindow *>(info);

    if (result != TEResultSuccess)
    {
        int i = 32;
    }
    switch (event)
    {
    case TEEventInstanceDidLoad:
        window->didLoad();
        break;
    case TEEventParameterLayoutDidChange:
        window->parameterLayoutDidChange();
        break;
    case TEEventFrameDidFinish:
        window->endFrame(start_time_value, start_time_scale, result);
        break;
	case TEEventGeneral:
		// TODO: check result here
		break;
    default:
        break;
    }
}

void DocumentWindow::parameterValueCallback(TEInstance * instance, const char *identifier, void * info)
{
    DocumentWindow *doc = static_cast<DocumentWindow *>(info);

	TEParameterInfo *param = nullptr;
	TEResult result = TEInstanceParameterGetInfo(instance, identifier, &param);
    if (result == TEResultSuccess && param->scope == TEScopeOutput)
    {
		switch (param->type)
		{
		case TEParameterTypeDouble:
		{
			double value;
			result = TEInstanceParameterGetDoubleValue(doc->myInstance, identifier, TEParameterValueCurrent, &value, 1);
			break;
		}
		case TEParameterTypeInt:
		{
			int32_t value;
			result = TEInstanceParameterGetIntValue(doc->myInstance, identifier, TEParameterValueCurrent, &value, 1);
			break;
		}
		case TEParameterTypeString:
		{
			TEString *value;
			result = TEInstanceParameterGetStringValue(doc->myInstance, identifier, TEParameterValueCurrent, &value);
			if (result == TEResultSuccess)
			{
				// Use value->string here
				TERelease(value);
			}
			break;
		}
		case TEParameterTypeTexture:
		{
            // Stash the state, we don't do any actual renderer work from this thread
            std::lock_guard<std::mutex> guard(doc->myMutex);
            doc->myPendingOutputTextures.push_back(identifier);
			break;
		}
		case TEParameterTypeFloatBuffer:
		{
			TEFloatBuffer *buffer = nullptr;
			result = TEInstanceParameterGetFloatBufferValue(doc->myInstance, identifier, TEParameterValueCurrent, &buffer);

			if (result == TEResultSuccess)
			{
				if (buffer && TEFloatBufferGetChannelCount(buffer) > 0 && TEFloatBufferGetValueCount(buffer) > 0)
				{
					auto data = TEFloatBufferGetValues(buffer);
					// we just grab the first sample in the first channel
					doc->myLastStreamValue = data[0][0];
					TERelease(&buffer);
				}
				else
				{
					doc->myLastStreamValue = 0.0;
				}
			}			
		}
		break;
		default:
			break;
		}
    }
	TERelease(&param);
}

void DocumentWindow::endFrame(int64_t time_value, int32_t time_scale, TEResult result)
{
    myInFrame = false;
}

DocumentWindow::DocumentWindow(std::wstring path, Mode mode)
    : myPath(path), myMode(mode), myInstance(nullptr), myWindow(nullptr),
	myRenderer(mode == Mode::DirectX ? static_cast<std::unique_ptr<Renderer>>(std::make_unique<DirectXRenderer>()) : static_cast<std::unique_ptr<Renderer>>(std::make_unique<OpenGLRenderer>())),
	myDidLoad(false), myInFrame(false), myLastStreamValue(1.0f), myLastFloatValue(0.0), myPendingLayoutChange(false)
{
}


DocumentWindow::~DocumentWindow()
{
	if (myInstance)
	{
		TERelease(&myInstance);
	}

    myRenderer->stop();

    if (myWindow)
    {
        PostMessageW(myWindow, WM_CLOSE, 0, 0);
    }
}

const std::wstring DocumentWindow::getPath() const
{
	return myPath;
}

void DocumentWindow::openWindow(HWND parent)
{
    RECT rc;
    SetRect(&rc, 0, 0, 640, 480);
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

    std::wstring title = getPath();
    if (getMode() == Mode::DirectX)
    {
        title += L" (DirectX)";
    }
    else
    {
        title += L" (OpenGL)";
    }
    myWindow = CreateWindowW(WindowClassName,
        title.data(),
        WS_OVERLAPPEDWINDOW | myRenderer->getWindowStyleFlags(),
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
        result = myRenderer->setup(myWindow) ? S_OK : EIO;
    }
	if (SUCCEEDED(result))
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::string utf8 = converter.to_bytes(getPath());

		TEResult TEResult = TEInstanceCreate(utf8.c_str(), TETimeInternal, eventCallback, parameterValueCallback, this, &myInstance);
		if (result == TEResultSuccess)
		{
			result = TEInstanceAssociateGraphicsContext(myInstance, myRenderer->getTEContext());
		}
		if (result == TEResultSuccess)
		{
			result = TEInstanceResume(myInstance);
		}
		assert(result == TEResultSuccess);

        SetTimer(myWindow, RenderTimerID, 16, nullptr);
	}
}

void DocumentWindow::parameterLayoutDidChange()
{
    myPendingLayoutChange = true;    
}

void DocumentWindow::render()
{
    // Make any pending renderer state updates
    if (myPendingLayoutChange)
    {
        myPendingLayoutChange = false;
        applyLayoutChange();
    }

    applyOutputTextureChange();

    float color[4] = { myDidLoad ? 0.6f : 0.6f, myDidLoad ? 0.6f : 0.6f, myDidLoad ? 1.0f : 0.6f, 1.0f};
    if (myDidLoad && !myInFrame)
    {
		TEStringArray *groups;
		TEResult result = TEInstanceGetParameterGroups(myInstance, TEScopeInput, &groups);
        if (result == TEResultSuccess)
        {
            int textureCount = 0;
            for (int32_t i = 0; i < groups->count; i++)
            {
				TEStringArray *children;
				result = TEInstanceParameterGetChildren(myInstance, groups->strings[i], &children);
				if (result == TEResultSuccess)
                {
                    for (int32_t j = 0; j < children->count; j++)
                    {
						TEParameterInfo *info;
						result = TEInstanceParameterGetInfo(myInstance, children->strings[j], &info);
						if (result == TEResultSuccess)
                        {
                            switch (info->type)
                            {
                            case TEParameterTypeDouble:
							{
								double d = fmod(myLastFloatValue, 1.0);
								result = TEInstanceParameterSetDoubleValue(myInstance, info->identifier, &d, 1);
								break;
							}
                            case TEParameterTypeInt:
							{
								int v = static_cast<int>(myLastFloatValue * 00) % 100;
								result = TEInstanceParameterSetIntValue(myInstance, info->identifier, &v, 1);
								break;
							}
                            case TEParameterTypeString:
                                result = TEInstanceParameterSetStringValue(myInstance, info->identifier, "test input");
                                break;
                            case TEParameterTypeTexture:
                            {
                                TED3D11Texture *texture = myRenderer->createLeftSideImage(textureCount);
								if (texture)
								{
									result = TEInstanceParameterSetTextureValue(myInstance, info->identifier, texture, myRenderer->getTEContext());
									TERelease(&texture);
								}
                                textureCount++;
                                break;
                            }
                            case TEParameterTypeFloatBuffer:
                            {
								TEFloatBuffer *buffer = nullptr;
								// Creating a copy of an existing buffer is more efficient than creating a new one every time
								result = TEInstanceParameterGetFloatBufferValue(myInstance, info->identifier, TEParameterValueCurrent, &buffer);
								if (result == TEResultSuccess)
								{
									// You might want to check more properties of the buffer than this
									if (buffer && TEFloatBufferGetCapacity(buffer) < 1 || TEFloatBufferGetChannelCount(buffer) != 2)
									{
										TERelease(&buffer);
									}
									if (buffer)
									{
										auto copied = TEFloatBufferCreateCopy(buffer);
										TERelease(&buffer);
										buffer = copied;
									}
									else
									{
										// Two channels, capacity of one sample per channel, no channel names
										// This buffer is not time-dependent, see TEFloatBuffer.h for handling time-dependent samples such
										// as audio.
										buffer = TEFloatBufferCreate(2, 1, nullptr);
									}
									float value = static_cast<float>(fmod(myLastFloatValue, 1.0));
									std::array<const float *, 2> channels{ &value, &value };
									TEFloatBufferSetValues(buffer, channels.data(), 1);

									result = TEInstanceParameterSetFloatBufferValue(myInstance, info->identifier, buffer);

									TERelease(&buffer);
								}								
                                break;
                            }
                            default:
                                break;
                            }
							TERelease(&info);
                        }
                        
                    }
					TERelease(&children);
                }
            }
        }

        if (TEInstanceStartFrameAtTime(myInstance, myTime, TimeRate, false) == TEResultSuccess)
        {
            myInFrame = true;
            myLastFloatValue += 1.0/(60.0 * 8.0);
        }
        else
        {
            myLastStreamValue = 1.0;
            color[0] = 1.0f;
            color[1] = color[2] = 0.2f;
        }
    }

    float intensity = (myLastStreamValue + 1.0f) / 2.0f;
    myRenderer->setBackgroundColor(color[0] * intensity, color[1] * intensity, color[2] * intensity);
    myRenderer->render();

	myTime += 100;
}

void DocumentWindow::cancelFrame()
{
    if (myInFrame)
    {
        TEResult result = TEInstanceCancelFrame(myInstance);
        if (result != TEResultSuccess)
        {
            // TODO: post it
        }
    }
}

void DocumentWindow::applyLayoutChange()
{
    myRenderer->clearLeftSideImages();
    myRenderer->clearRightSideImages();
    myOutputParameterTextureMap.clear();

    for (auto scope : { TEScopeInput, TEScopeOutput })
    {
        TEStringArray *groups;
        TEResult result = TEInstanceGetParameterGroups(myInstance, scope, &groups);
        if (result == TEResultSuccess)
        {
            for (int32_t i = 0; i < groups->count; i++)
            {
                TEParameterInfo *group;
                result = TEInstanceParameterGetInfo(myInstance, groups->strings[i], &group);
                if (result == TEResultSuccess)
                {
                    // Use group info here
                    TERelease(&group);
                }
                TEStringArray *children = nullptr;
                if (result == TEResultSuccess)
                {
                    result = TEInstanceParameterGetChildren(myInstance, groups->strings[i], &children);
                }
                if (result == TEResultSuccess)
                {
                    for (int32_t j = 0; j < children->count; j++)
                    {
                        TEParameterInfo *info;
                        result = TEInstanceParameterGetInfo(myInstance, children->strings[j], &info);
                        if (result == TEResultSuccess)
                        {
                            if (result == TEResultSuccess && info->type == TEParameterTypeTexture)
                            {
                                if (scope == TEScopeInput)
                                {
                                    std::array<unsigned char, 256 * 256 * 4> tex;

                                    for (int y = 0; y < 256; y++)
                                    {
                                        for (int x = 0; x < 256; x++)
                                        {
                                            tex[(y * 256 * 4) + (x * 4) + 0] = x;
                                            tex[(y * 256 * 4) + (x * 4) + 1] = 0;
                                            tex[(y * 256 * 4) + (x * 4) + 2] = getMode() == Mode::OpenGL ? 255 - y : y;
                                            tex[(y * 256 * 4) + (x * 4) + 3] = 255;
                                        }
                                    }
                                    myRenderer->addLeftSideImage(tex.data(), 256 * 4, 256, 256);
                                }
                                else
                                {
                                    myRenderer->addRightSideImage();
                                    myOutputParameterTextureMap[info->identifier] = myRenderer->getRightSideImageCount() - 1;
                                }
                            }
                            TERelease(&info);
                        }
                    }
                    TERelease(&children);
                }
            }
        }
    }
}

void DocumentWindow::applyOutputTextureChange()
{
    // Only hold the lock briefly
    std::vector<std::string> changes;
    {
        std::lock_guard<std::mutex> guard(myMutex);
        std::swap(myPendingOutputTextures, changes);
    }

    for (const auto & identifier : changes)
    {
        TETexture *texture = nullptr;
        TEResult result = TEInstanceParameterGetTextureValue(myInstance, identifier.c_str(), TEParameterValueCurrent, &texture);
        if (result == TEResultSuccess)
        {
            size_t imageIndex = myOutputParameterTextureMap[identifier];

            myRenderer->setRightSideImage(imageIndex, texture);

            if (texture)
            {
                TERelease(&texture);
            }
        }
    }
}
