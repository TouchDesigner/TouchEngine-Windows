#include "stdafx.h"
#include <commdlg.h>
#include "DocumentWindow.h"
#include "Resource.h"
#include "DirectXRenderer.h"
#include "OpenGLRenderer.h"
#include <codecvt>
#include <vector>
#include <array>

wchar_t *DocumentWindow::WindowClassName = L"DocumentWindow";
const int32_t DocumentWindow::InputChannelCount = 2;
const double DocumentWindow::InputSampleRate = 44100.0;
const int64_t DocumentWindow::InputSampleLimit = 44100 / 2;
const int64_t DocumentWindow::InputSamplesPerFrame = 44100 / 60;

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
	LoadStringW(hInstance, IDC_TDPTESTHOST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	DocumentWindow::registerClass(hInst);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TDPTESTHOST));

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
		else if (theOpenDocument)
		{
			theOpenDocument->render();
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TDPTESTHOST));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TDPTESTHOST);
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
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return LRESULT();
}

void DocumentWindow::eventCallback(TPInstance * instance, TPEvent event, TPResult result, int64_t time_value, int32_t time_scale, void * info)
{
    DocumentWindow *window = static_cast<DocumentWindow *>(info);

    if (result != TPResultSuccess)
    {
        int i = 32;
    }
    switch (event)
    {
    case TPEventInstanceDidLoad:
        window->didLoad();
        break;
    case TPEventParameterLayoutDidChange:
        window->parameterLayoutDidChange();
        break;
    case TPEventFrameDidFinish:
        window->endFrame(time_value, time_scale, result);
        break;
    default:
        break;
    }
}

void DocumentWindow::parameterValueCallback(TPInstance * instance, const char *identifier, void * info)
{
    DocumentWindow *doc = static_cast<DocumentWindow *>(info);

	TPParameterInfo *param = nullptr;
	TPResult result = TPInstanceParameterCopyInfo(instance, identifier, &param);
    if (result == TPResultSuccess && param->scope == TPScopeOutput)
    {
		switch (param->type)
		{
		case TPParameterTypeDouble:
		{
			double value;
			result = TPInstanceParameterGetDoubleValue(doc->myInstance, identifier, TPParameterValueCurrent, &value, 1);
			break;
		}
		case TPParameterTypeInt:
		{
			int32_t value;
			result = TPInstanceParameterGetIntValue(doc->myInstance, identifier, TPParameterValueCurrent, &value, 1);
			break;
		}
		case TPParameterTypeString:
		{
			TPString *value;
			result = TPInstanceParameterCopyStringValue(doc->myInstance, identifier, TPParameterValueCurrent, &value);
			if (result == TPResultSuccess)
			{
				// Use value->string here
				TPRelease(value);
			}
			break;
		}
		case TPParameterTypeTexture:
		{
			TPTexture *texture = nullptr;
			result = TPInstanceParameterCopyTextureValue(doc->myInstance, identifier, TPParameterValueCurrent, &texture);
			if (result == TPResultSuccess)
			{
				size_t imageIndex = doc->myOutputParameterTextureMap[identifier];

				doc->myRenderer->setRightSideImage(imageIndex, texture);

				if (texture)
				{
					TPRelease(&texture);
				}
			}
			break;
		}
		case TPParameterTypeFloatStream:
		{

			TPStreamDescription *desc = nullptr;
			result = TPInstanceParameterCopyStreamDescription(doc->myInstance, identifier, &desc);

			if (result == TPResultSuccess)
			{
				int32_t channelCount = desc->numChannels;
				std::vector <std::vector<float>> store(channelCount);
				std::vector<float *> channels;

				int64_t maxSamples = desc->maxSamples;
				for (auto &vector : store)
				{
					vector.resize(maxSamples);
					channels.emplace_back(vector.data());
				}

				int64_t length = maxSamples;
				result = TPInstanceParameterGetOutputStreamValues(doc->myInstance, identifier, channels.data(), int32_t(channels.size()), &length);
				if (result == TPResultSuccess)
				{
					// Use the channel data here
					if (length > 0 && channels.size() > 0)
					{
						doc->myLastStreamValue = store.back()[length - 1];
					}
				}
				TPRelease(&desc);
			}
			
		}
		break;
		default:
			break;
		}
    }
	TPRelease(&param);
}

void DocumentWindow::endFrame(int64_t time_value, int32_t time_scale, TPResult result)
{
    myInFrame = false;
}

DocumentWindow::DocumentWindow(std::wstring path, Mode mode)
    : myPath(path), myInstance(nullptr), myWindow(nullptr),
	myRenderer(mode == Mode::DirectX ? static_cast<std::unique_ptr<Renderer>>(std::make_unique<DirectXRenderer>()) : static_cast<std::unique_ptr<Renderer>>(std::make_unique<OpenGLRenderer>())),
	myDidLoad(false), myInFrame(false), myLastStreamValue(1.0f), myLastFloatValue(0.0)
{
}


DocumentWindow::~DocumentWindow()
{
	// TODO: we shouldn't have to do this but TPTextures are holding on to the connection which gets invalidated
	myRenderer->clearLeftSideImages();
	myRenderer->clearRightSideImages();

	if (myInstance)
	{
		TPRelease(&myInstance);
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

    std::wstring path = getPath();
    myWindow = CreateWindowW(WindowClassName,
        path.data(),
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
		std::string utf8 = converter.to_bytes(path);
		TPResult tpResult = TPInstanceCreate(utf8.c_str(), TPTimeInternal, eventCallback, parameterValueCallback, this, &myInstance);
	}
}

void DocumentWindow::parameterLayoutDidChange()
{
    myRenderer->clearLeftSideImages();
    myRenderer->clearRightSideImages();
    myOutputParameterTextureMap.clear();

    for (auto scope : { TPScopeInput, TPScopeOutput })
    {
        TPStringArray *groups;
		TPResult result = TPInstanceCopyParameterGroups(myInstance, scope, &groups);
        if (result == TPResultSuccess)
        {
            for (int32_t i = 0; i < groups->count; i++)
            {
				TPParameterInfo *group;
				result = TPInstanceParameterCopyInfo(myInstance, groups->strings[i], &group);
				if (result == TPResultSuccess)
				{
					// Use group info here
					TPRelease(&group);
				}
				TPStringArray *children = nullptr;
				if (result == TPResultSuccess)
				{
					result = TPInstanceParameterCopyChildren(myInstance, groups->strings[i], &children);
				}
                if (result == TPResultSuccess)
                {
                    for (int32_t j = 0; j < children->count; j++)
                    {
						TPParameterInfo *info;
						result = TPInstanceParameterCopyInfo(myInstance, children->strings[j], &info);
						if (result == TPResultSuccess)
						{
							if (info->type == TPParameterTypeFloatStream && scope == TPScopeInput)
							{
								TPStreamDescription desc;
								desc.rate = InputSampleRate;
								desc.numChannels = InputChannelCount;
								desc.maxSamples = InputSampleLimit;
								result = TPInstanceParameterSetInputStreamDescription(myInstance, info->identifier, &desc);
							}
							else if (result == TPResultSuccess && info->type == TPParameterTypeTexture)
							{
								if (scope == TPScopeInput)
								{
									std::array<unsigned char, 256 * 256 * 4> tex;

									for (int y = 0; y < 256; y++)
									{
										for (int x = 0; x < 256; x++)
										{
											tex[(y * 256 * 4) + (x * 4) + 0] = x;
											tex[(y * 256 * 4) + (x * 4) + 1] = 40;
											tex[(y * 256 * 4) + (x * 4) + 2] = y;
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
							TPRelease(&info);
						}
                    }
					TPRelease(&children);
                }
            }
        }
    }    
}

void DocumentWindow::render()
{
    float color[4] = { myDidLoad ? 0.6f : 0.6f, myDidLoad ? 0.6f : 0.6f, myDidLoad ? 1.0f : 0.6f, 1.0f};
    if (myDidLoad && !myInFrame)
    {
		TPStringArray *groups;
		TPResult result = TPInstanceCopyParameterGroups(myInstance, TPScopeInput, &groups);
        if (result == TPResultSuccess)
        {
            int textureCount = 0;
            for (int32_t i = 0; i < groups->count; i++)
            {
				TPStringArray *children;
				result = TPInstanceParameterCopyChildren(myInstance, groups->strings[i], &children);
				if (result == TPResultSuccess)
                {
                    for (int32_t j = 0; j < children->count; j++)
                    {
						TPParameterInfo *info;
						result = TPInstanceParameterCopyInfo(myInstance, children->strings[j], &info);
						if (result == TPResultSuccess)
                        {
                            switch (info->type)
                            {
                            case TPParameterTypeDouble:
							{
								double d = fmod(myLastFloatValue, 1.0);
								result = TPInstanceParameterSetDoubleValue(myInstance, info->identifier, &d, 1);
								break;
							}
                            case TPParameterTypeInt:
							{
								int v = static_cast<int>(myLastFloatValue * 00) % 100;
								result = TPInstanceParameterSetIntValue(myInstance, info->identifier, &v, 1);
								break;
							}
                            case TPParameterTypeString:
                                result = TPInstanceParameterSetStringValue(myInstance, info->identifier, "test input");
                                break;
                            case TPParameterTypeTexture:
                            {
                                TPD3DTexture *texture = myRenderer->createLeftSideImage(textureCount);
								if (texture)
								{
									result = TPInstanceParameterSetTextureValue(myInstance, info->identifier, texture);
									TPRelease(&texture);
								}

                                textureCount++;
                                break;
                            }
                            case TPParameterTypeFloatStream:
                            {
                                std::array<float, InputSamplesPerFrame> channel;
                                std::fill(channel.begin(), channel.end(), static_cast<float>(fmod(myLastFloatValue, 1.0)));
                                std::array<const float *, InputChannelCount> channels;
                                std::fill(channels.begin(), channels.end(), channel.data());
                                int64_t filled = channel.size();
                                result = TPInstanceParameterAppendStreamValues(myInstance, info->identifier, channels.data(), static_cast<int32_t>(channels.size()), &filled);
                                break;
                            }
                            default:
                                break;
                            }
							TPRelease(&info);
                        }
                        
                    }
					TPRelease(&children);
                }
            }
        }

        if (TPInstanceStartFrameAtTime(myInstance, 1000, 1000 * 1000) == TPResultSuccess)
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
}

void DocumentWindow::cancelFrame()
{
    if (myInFrame)
    {
        TPResult result = TPInstanceCancelFrame(myInstance);
        if (result != TPResultSuccess)
        {
            // TODO: post it
        }
    }
}
