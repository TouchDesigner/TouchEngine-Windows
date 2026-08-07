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
#include "Application.h"
#include "DocumentWindow.h"
#include "Resource.h"
#include "Strings.h"
#include <commdlg.h>

constexpr int MAX_LOADSTRING = 100;

static WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
static WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

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

	// Perform application initialization:
	Application app(hInstance);
	if (!app.openWindow(nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TETESTHOST));

	MSG msg{};

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

Application::Application(HINSTANCE hInstance)
	: myInstance(hInstance),
	myWindow(hInstance, *this)
{
	
}

bool Application::openWindow(int nCmdShow)
{
	myWindow.open(nCmdShow);

	return true;
}

HINSTANCE Application::getInstance() const
{
	return myInstance;
}

void Application::about()
{
	DialogBox(myInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), myWindow.getHWND(), dialogProc);
}

void Application::exit()
{
	myWindow.close();
	PostQuitMessage(0);
}

void Application::openDocument(Graphics api)
{
	myDocument.emplace(*this, api);

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
		std::string path = ConvertToMultiByte(buffer);
		myDocument->open(path);
	}
	else
	{
		myDocument.reset();
	}
}

const Window& Application::getWindow() const
{
	return myWindow;
}

void Application::documentWindowDidClose()
{
	myDocument = std::nullopt;
}

INT_PTR Application::dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

Application::AppWindow::Config Application::AppWindow::AppWindowConfig = {
	szWindowClass,
	MAKEINTRESOURCEW(IDC_TETESTHOST),
	Window::Config::Background::ColorWindow,
	InitialWindowWidth,
	InitialWindowHeight
};

Application::AppWindow::AppWindow(HINSTANCE instance, Application& app)
	: Window(AppWindowConfig, instance, szTitle, Window(), 0),
	myApp(app)
{

}

bool Application::AppWindow::command(int wmId)
{
	switch (wmId)
	{
	case IDM_ABOUT:
		myApp.about();
		return true;
	case IDM_EXIT:
		myApp.exit();
		return true;
	case ID_FILE_OPEN_DX11:
		myApp.openDocument(Graphics::DX11);
		return true;
	case ID_FILE_OPEN_DX12:
		myApp.openDocument(Graphics::DX12);
		return true;
	case ID_FILE_OPENOPENGL:
		myApp.openDocument(Graphics::OpenGL);
		return true;
	default:
		break;
	}
	return false;
}

bool Application::AppWindow::paint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(getHWND(), &ps);

	EndPaint(getHWND(), &ps);
	return true;
}

void Application::AppWindow::destroy()
{
	Window::destroy();
	myApp.exit();
}
