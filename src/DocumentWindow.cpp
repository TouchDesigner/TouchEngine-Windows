/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include <commdlg.h>
#include "DocumentWindow.h"
#include "Resource.h"
#include "DX11Renderer.h"
#include "DX12Renderer.h"
#include "OpenGLRenderer.h"
#include <codecvt>
#include <array>

wchar_t *DocumentWindow::WindowClassName = L"DocumentWindow";
const int32_t DocumentWindow::InputChannelCount = 2;
const double DocumentWindow::InputSampleRate = 44100.0;
const int64_t DocumentWindow::InputSampleLimit = 44100 / 2;
const int64_t DocumentWindow::InputSamplesPerFrame = 44100 / 60;
const UINT_PTR DocumentWindow::UpdateTimerID = 1;

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
ATOM
MyRegisterClass(HINSTANCE hInstance)
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
BOOL
InitInstance(HINSTANCE hInstance, int nCmdShow)
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
LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		case ID_FILE_OPEN_DX11:
			theOpenDocument = Open(hWnd, DocumentWindow::Mode::DirectX11);
			break;
		case ID_FILE_OPEN_DX12:
			theOpenDocument = Open(hWnd, DocumentWindow::Mode::DirectX12);
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
INT_PTR CALLBACK
About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
	ofns.lpstrTitle = L"Select a file to open";
	ofns.lpstrFilter = _T("All Files\0*.*\0TouchDesigner Components\0*.TOX\0");
	ofns.nFilterIndex = 2;
	BOOL result = GetOpenFileName(&ofns);
	if (result)
	{
		std::shared_ptr<DocumentWindow> win(std::make_shared<DocumentWindow>(buffer, mode));
		win->openWindow(hWnd);

		return win;
	}
	return std::shared_ptr<DocumentWindow>();
}


HRESULT
DocumentWindow::registerClass(HINSTANCE hInstance)
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

LRESULT CALLBACK
DocumentWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
	{
		KillTimer(hWnd, UpdateTimerID);
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
	case WM_SIZE:
	{
		if (theOpenDocument && theOpenDocument->getWindow() == hWnd)
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			theOpenDocument->myRenderer->resize(width, height);
			theOpenDocument->update();
		}
		break;
	}
	case WM_TIMER:
	{
		if (wParam == UpdateTimerID)
		{
			if (theOpenDocument)
			{
				theOpenDocument->update();
			}
		}

		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return LRESULT();
}

void DocumentWindow::didConfigure(TEResult result)
{
	// Configuration can be cancelled by a subsequent configuration or other action
	// - we can ignore the event in that case and await a following one
	if (result != TEResultCancelled)
	{
		std::lock_guard<std::mutex> guard(myMutex);
		myConfigureRenderer = true;
		myConfigureResult = result;
	}
}

void
DocumentWindow::eventCallback(TEInstance * instance,
									TEEvent event,
									TEResult result,
									int64_t start_time_value,
									int32_t start_time_scale,
									int64_t end_time_value,
									int32_t end_time_scale,
									void * info)
{
	DocumentWindow *window = static_cast<DocumentWindow *>(info);

	switch (event)
	{
	case TEEventInstanceReady:
		window->didConfigure(result);
		break;
	case TEEventInstanceDidLoad:
		window->didLoad();
		break;
	case TEEventInstanceDidUnload:
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

void
DocumentWindow::linkEventCallback(TEInstance * instance, TELinkEvent event, const char *identifier, void * info)
{
	DocumentWindow* doc = static_cast<DocumentWindow*>(info);
	switch (event)
	{
	case TELinkEventAdded:
		doc->linkLayoutDidChange();
		break;
	case TELinkEventValueChange:
		doc->linkValueChange(identifier);
		break;
	default:
		break;
	}
}

void
DocumentWindow::linkValueChange(const char* identifier)
{
	TouchObject<TELinkInfo> link;
	TEResult result = TEInstanceLinkGetInfo(myInstance, identifier, link.take());
	if (result == TEResultSuccess && link->scope == TEScopeOutput)
	{
		switch (link->type)
		{
		case TELinkTypeDouble:
		{
			double value;
			result = TEInstanceLinkGetDoubleValue(myInstance, identifier, TELinkValueCurrent, &value, 1);
			break;
		}
		case TELinkTypeInt:
		{
			int32_t value;
			result = TEInstanceLinkGetIntValue(myInstance, identifier, TELinkValueCurrent, &value, 1);
			break;
		}
		case TELinkTypeString:
		{
			TouchObject<TEString> value;
			result = TEInstanceLinkGetStringValue(myInstance, identifier, TELinkValueCurrent, value.take());
			if (result == TEResultSuccess)
			{
				// Use value->string here
			}
			break;
		}
		case TELinkTypeTexture:
		{
			// Stash the state, we don't do any actual renderer work from this thread
			std::lock_guard<std::mutex> guard(myMutex);
			myPendingOutputTextures.push_back(identifier);
			break;
		}
		case TELinkTypeFloatBuffer:
		{
			TouchObject<TEFloatBuffer> buffer;
			result = TEInstanceLinkGetFloatBufferValue(myInstance, identifier, TELinkValueCurrent, buffer.take());

			if (result == TEResultSuccess)
			{
				uint32_t valueCount = TEFloatBufferGetValueCount(buffer);
				int32_t channelCount = TEFloatBufferGetChannelCount(buffer);
				if (buffer && channelCount > 0 && valueCount > 0)
				{
					const float * const *data = TEFloatBufferGetValues(buffer);

					for (int channel = 0; channel < channelCount; channel++)
					{
						// Here we just grab the first sample in the channel
						float value = data[channel][0];
					}
				}
			}
			break;
		}
		case TELinkTypeStringData:
		{
			TouchObject<TEObject> value;
			result = TEInstanceLinkGetObjectValue(myInstance, identifier, TELinkValueCurrent, value.take());
			// String data can be a TETable or TEString, so check the type
			if (value && TEGetType(value) == TEObjectTypeTable)
			{
				TouchObject<TETable> table;
				table.set(static_cast<TETable*>(value.get()));
				// do something with the table
			}
			else if (value && TEGetType(value) == TEObjectTypeString)
			{
				TouchObject<TEString> string;
				string.set(static_cast<TEString*>(value.get()));
				// do something with the string
			}
			break;
		}
		default:
			break;
		}
	}
}

void
DocumentWindow::endFrame(int64_t time_value, int32_t time_scale, TEResult result)
{
	setInFrame(false);
}

void
DocumentWindow::getState(bool& configured, bool& loaded, bool& linksChanged, bool& inFrame)
{
	std::lock_guard<std::mutex> guard(myMutex);
	configured = myConfigureRenderer;
	myConfigureRenderer = false;
	loaded = myDidLoad;
	if (myDidLoad)
	{
		// For this example, we are only interested in links after load has completed
		linksChanged = myPendingLayoutChange;
		inFrame = myInFrame;
		myPendingLayoutChange = false;
	}
	else
	{
		linksChanged = false;
		inFrame = false;
	}
}

void
DocumentWindow::setInFrame(bool inFrame)
{
	std::lock_guard<std::mutex> guard(myMutex);
	myInFrame = inFrame;
}

DocumentWindow::DocumentWindow(std::wstring path, Mode mode)
	: myPath(path), myMode(mode)
{
	switch (mode)
	{
	case Mode::DirectX11:
		myRenderer = static_cast<std::unique_ptr<Renderer>>(std::make_unique<DX11Renderer>());
		break;
	case Mode::DirectX12:
		myRenderer = static_cast<std::unique_ptr<Renderer>>(std::make_unique<DX12Renderer>());
		break;
	default:
		myRenderer = static_cast<std::unique_ptr<Renderer>>(std::make_unique<OpenGLRenderer>());
		break;
	}
}


DocumentWindow::~DocumentWindow()
{
	// Do this first so releated resources our Renderer may be interested in are released
	myInstance.reset();

	myRenderer->stop();

	if (myWindow)
	{
		PostMessageW(myWindow, WM_CLOSE, 0, 0);
	}
}

const std::wstring
DocumentWindow::getPath() const
{
	return myPath;
}

void
DocumentWindow::openWindow(HWND parent)
{
	RECT rc;
	SetRect(&rc, 0, 0, InitialWindowWidth, InitialWindowHeight);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	std::wstring title = getPath();
	switch (getMode())
	{
	case Mode::DirectX11:
		title += L" (DirectX 11)";
		break;
	case Mode::DirectX12:
		title += L" (DirectX 12)";
		break;
	default:
		title += L" (OpenGL)";
		break;
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
		myRenderer->resize(InitialWindowWidth, InitialWindowHeight);
	}
	if (SUCCEEDED(result))
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::string utf8 = converter.to_bytes(getPath());

		TEResult teresult = TEInstanceCreate(eventCallback, linkEventCallback, this, myInstance.take());
		if (teresult == TEResultSuccess)
		{
			teresult = TEInstanceAssociateGraphicsContext(myInstance, myRenderer->getTEContext());
		}
		if (teresult == TEResultSuccess)
		{
			teresult = TEInstanceConfigure(myInstance, utf8.c_str(), TETimeExternal);
		}
		if (teresult == TEResultSuccess)
		{
			teresult = TEInstanceLoad(myInstance);
		}
		if (teresult == TEResultSuccess)
		{
			teresult = TEInstanceResume(myInstance);
		}
		assert(teresult == TEResultSuccess);

		SetTimer(myWindow, UpdateTimerID, static_cast<UINT>(std::ceil(1000. / FramesPerSecond)), nullptr);

		// Draw once
		render(false);
	}
}

void
DocumentWindow::linkLayoutDidChange()
{
	std::lock_guard<std::mutex> guard(myMutex);
	myPendingLayoutChange = true;    
}

void
DocumentWindow::update()
{
	bool configured, loaded, linksChanged, inFrame;
	getState(configured, loaded, linksChanged, inFrame);

	if (configured)
	{
		std::wstring message;
		if (TEResultGetSeverity(myConfigureResult) == TESeverityError)
		{			
			const char *description = TEResultGetDescription(myConfigureResult);
			
			message = L"There was an error configuring TouchEngine: ";
			if (description)
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

				message += converter.from_bytes(description);
			}
			else
			{
				message += std::to_wstring(myConfigureResult);
			}
			
			myConfigureError = true;
		}
		else
		{
			myConfigureError = !myRenderer->configure(myInstance, message);
		}
		if (myConfigureError)
		{
			MessageBox(myWindow, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
			SendMessage(myWindow, WM_CLOSE, 0, 0);
		}
	}

	if (myConfigureError)
	{
		return;
	}

	bool changed = linksChanged;

	// Make any pending renderer state updates
	if (linksChanged)
	{
		applyLayoutChange();
	}

	if (loaded && !inFrame)
	{
		changed = changed || applyOutputTextureChange();

		// Examples of setting input links
		TouchObject<TEStringArray> groups;
		TEResult result = TEInstanceGetLinkGroups(myInstance, TEScopeInput, groups.take());
		if (result == TEResultSuccess)
		{
			int textureCount = 0;
			for (int32_t i = 0; i < groups->count; i++)
			{
				TouchObject<TEStringArray> children;
				result = TEInstanceLinkGetChildren(myInstance, groups->strings[i], children.take());
				if (result == TEResultSuccess)
				{
					for (int32_t j = 0; j < children->count; j++)
					{
						TouchObject<TELinkInfo> info;
						result = TEInstanceLinkGetInfo(myInstance, children->strings[j], info.take());
						if (result == TEResultSuccess)
						{
							switch (info->type)
							{
							case TELinkTypeDouble:
							{
								double d = fmod(myLastFloatValue, 1.0);
								result = TEInstanceLinkSetDoubleValue(myInstance, info->identifier, &d, 1);
								break;
							}
							case TELinkTypeInt:
							{
								int v = static_cast<int>(myLastFloatValue * 100) % 100;
								result = TEInstanceLinkSetIntValue(myInstance, info->identifier, &v, 1);
								break;
							}
							case TELinkTypeString:
								result = TEInstanceLinkSetStringValue(myInstance, info->identifier, "test input");
								break;
							case TELinkTypeTexture:
							{
								TouchObject<TETexture> texture;
								TouchObject<TESemaphore> semaphore;
								uint64_t waitValue = 0;
								// Our OpenGL and D3D11 renderers use their TEGraphicsContexts to handle setting inputs, meaning they needn't do any sync themselves
								// - but at the cost of a texture copy by the TEGraphicsContext
								// Our D3D12 renderer creates shareable textures, so it must handle sync itself - when setting a texture we uses a texture transfer
								// to supply a fence and wait-value to the instance - the instance will insert a wait for the fence prior to consuming the input texture
								if (myRenderer->getInputImage(textureCount, texture, semaphore, waitValue))
								{
									result = TEInstanceLinkSetTextureValue(myInstance, info->identifier, texture, myRenderer->getTEContext());
									if (result == TEResultSuccess && myRenderer->doesTextureTransfer())
									{
										result = TEInstanceAddTextureTransfer(myInstance, texture, semaphore, waitValue);
									}
								}
								textureCount++;
								break;
							}
							case TELinkTypeFloatBuffer:
							{
								TouchObject<TEFloatBuffer> buffer;
								// Creating a copy of an existing buffer is more efficient than creating a new one every time
								result = TEInstanceLinkGetFloatBufferValue(myInstance, info->identifier, TELinkValueCurrent, buffer.take());
								if (result == TEResultSuccess)
								{
									// You might want to check more properties of the buffer than this
									if (buffer && TEFloatBufferGetCapacity(buffer) < 1 || TEFloatBufferGetChannelCount(buffer) != 2)
									{
										buffer.reset();
									}
									if (buffer)
									{
										TouchObject<TEFloatBuffer> copied;
										copied.take(TEFloatBufferCreateCopy(buffer));
										buffer = copied;
									}
									else
									{
										// Two channels, capacity of one sample per channel, no channel names
										// This buffer is not time-dependent, see TEFloatBuffer.h for handling time-dependent samples such
										// as audio.
										buffer.take(TEFloatBufferCreate(-1, 2, 1, nullptr));
									}
									float value = static_cast<float>(fmod(myLastFloatValue, 1.0));
									std::array<const float*, 2> channels{ &value, &value };
									TEFloatBufferSetValues(buffer, channels.data(), 1);

									result = TEInstanceLinkSetFloatBufferValue(myInstance, info->identifier, buffer);
								}
								break;
							}
							case TELinkTypeStringData:
							{
								// String data can be either tabular, in which case set a TETable, or a single string - here we set a table
								// (use TEInstanceLinkSetStringValue() to set a string value)

								// It is more efficient to create a copy of an existing table than to create a new one, so check
								// for an existing table to re-use first.
								TouchObject<TEObject> value;
								result = TEInstanceLinkGetObjectValue(myInstance, info->identifier, TELinkValueCurrent, value.take());

								if (result == TEResultSuccess)
								{
									TouchObject<TETable> table ;
									if (value && TEGetType(value) == TEObjectTypeTable)
									{
										table.take(TETableCreateCopy(static_cast<TETable*>(value.get())));
									}
									else
									{
										table.take(TETableCreate());
									}
									TETableResize(table, 3, 2);
									for (int column = 0; column < 2; column++)
									{
										for (int row = 0; row < 3; row++)
										{
											TETableSetStringValue(table, row, column, "test");
										}
									}
									result = TEInstanceLinkSetTableValue(myInstance, info->identifier, table);
								}
								break;
							}
							default:
								break;
							}
						}
					}
				}
			}
		}

		setInFrame(true);

		int64_t time = getRenderTime();
		myLastResult = TEInstanceStartFrameAtTime(myInstance, time, TimeRate, false);
		if (myLastResult == TEResultSuccess)
		{
			myLastFloatValue += 1.0 / (60.0 * 8.0);
		}
		else
		{
			setInFrame(false);
		}
	}
	if (changed)
	{
		render(loaded);
	}
}

void
DocumentWindow::render(bool loaded)
{
	float color[4] = { loaded ? 0.6f : 0.6f, loaded ? 0.6f : 0.6f, loaded ? 1.0f : 0.6f, 1.0f };

	if (myLastResult != TEResultSuccess)
	{
		color[0] = 1.0f;
		color[1] = color[2] = 0.2f;
	}

	myRenderer->setBackgroundColor(color[0], color[1], color[2]);
	myRenderer->render();
}

void
DocumentWindow::applyLayoutChange()
{
	myRenderer->beginImageLayout();

	myRenderer->clearInputImages();
	myRenderer->clearOutputImages();
	myOutputLinkTextureMap.clear();

	for (auto scope : { TEScopeInput, TEScopeOutput })
	{
		TouchObject<TEStringArray> groups;
		TEResult result = TEInstanceGetLinkGroups(myInstance, scope, groups.take());
		if (result == TEResultSuccess)
		{
			for (int32_t i = 0; i < groups->count; i++)
			{
				TouchObject<TELinkInfo> group;
				result = TEInstanceLinkGetInfo(myInstance, groups->strings[i], group.take());
				if (result == TEResultSuccess)
				{
					// Use group info here
				}
				TouchObject<TEStringArray> children;
				if (result == TEResultSuccess)
				{
					result = TEInstanceLinkGetChildren(myInstance, groups->strings[i], children.take());
				}
				if (result == TEResultSuccess)
				{
					for (int32_t j = 0; j < children->count; j++)
					{
						TouchObject<TELinkInfo> info;
						result = TEInstanceLinkGetInfo(myInstance, children->strings[j], info.take());
						if (result == TEResultSuccess)
						{
							if (result == TEResultSuccess && info->type == TELinkTypeTexture)
							{
								if (scope == TEScopeInput)
								{
									std::vector<unsigned char> tex( ImageWidth * ImageHeight * 4 );

									std::array<Gradient, 4> gradients{
										Gradient{{0, 0, 0}, {255,0,255}},
										Gradient{{100, 100, 100}, {255, 255, 0}},
										Gradient{{40, 40, 40}, {255, 255, 255}},
										Gradient{{255, 0, 0}, {255, 0, 255}}
									};

									const auto &gradient = gradients[myRenderer->getInputImageCount() % gradients.size()];
									auto& start = gradient.start;
									auto& end = gradient.end;
									for (size_t y = 0; y < ImageHeight; y++)
									{
										for (size_t x = 0; x < ImageWidth; x++)
										{
											double xColor = static_cast<double>(x) / (ImageWidth-1);
											double yColor = static_cast<double>(y) / (ImageHeight-1);
											if (getMode() == Mode::OpenGL)
												yColor = 1.0 - yColor;
											Color xColor1 = {
												start.red + static_cast<int>(yColor * (static_cast<double>(end.red) - start.red)),
												start.green + static_cast<int>(xColor * (static_cast<double>(end.green) - start.green)),
												start.blue + static_cast<int>(xColor * (static_cast<double>(end.blue) - start.blue))
											};
											tex[(y * ImageWidth * 4) + (x * 4) + 0] = xColor1.blue;
											tex[(y * ImageWidth * 4) + (x * 4) + 1] = xColor1.green;
											tex[(y * ImageWidth * 4) + (x * 4) + 2] = xColor1.red;
											tex[(y * ImageWidth * 4) + (x * 4) + 3] = 255;
										}
									}
									myRenderer->addInputImage(tex.data(), ImageWidth * 4, ImageWidth, ImageHeight);
								}
								else
								{
									myRenderer->addOutputImage();
									myOutputLinkTextureMap[info->identifier] = myRenderer->getRightSideImageCount() - 1;
								}
							}
						}
					}
				}
			}
		}
	}

	myRenderer->endImageLayout();
}

bool
DocumentWindow::applyOutputTextureChange()
{
	// Only hold the lock briefly
	std::vector<std::string> changes;
	{
		std::lock_guard<std::mutex> guard(myMutex);
		std::swap(myPendingOutputTextures, changes);
	}

	for (const auto & identifier : changes)
	{
		TouchObject<TETexture> texture;
		
		TEResult result = TEInstanceLinkGetTextureValue(myInstance, identifier.c_str(), TELinkValueCurrent, texture.take());
		if (result == TEResultSuccess)
		{
			size_t imageIndex = myOutputLinkTextureMap[identifier];

			TouchObject<TETexture> previous;
			TouchObject<TESemaphore> semaphore;
			uint64_t waitValue;

			if (myRenderer->releaseOutputImage(imageIndex, previous, semaphore, waitValue) && myRenderer->doesTextureTransfer())
			{
				TEInstanceAddTextureTransfer(myInstance, previous, semaphore, waitValue);
			}

			myRenderer->setOutputImage(imageIndex, texture);

			if (result == TEResultSuccess && myRenderer->doesTextureTransfer())
			{
				if (texture && TEInstanceHasTextureTransfer(myInstance, texture))
				{
					result = TEInstanceGetTextureTransfer(myInstance, texture, semaphore.take(), &waitValue);

					if (result == TEResultSuccess)
					{
						myRenderer->acquireOutputImage(imageIndex, semaphore, waitValue);
					}
				}
			}
		}
	}

	return !changes.empty();
}

int64_t
DocumentWindow::getRenderTime()
{
	LARGE_INTEGER now{ 0 };

	if (myStartTime.QuadPart == 0)
	{
		QueryPerformanceFrequency(&myPerformanceCounterFrequency);
		QueryPerformanceCounter(&myStartTime);
		now.QuadPart = 0;
	}
	else
	{
		QueryPerformanceCounter(&now);
		now.QuadPart -= myStartTime.QuadPart;
	}

	now.QuadPart *= TimeRate;
	now.QuadPart /= myPerformanceCounterFrequency.QuadPart;

	return now.QuadPart;
}
