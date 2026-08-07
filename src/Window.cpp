#include "stdafx.h"
#include "Window.h"
#include "Resource.h"
#include "Strings.h"
#include <commdlg.h>
#include <stdexcept>

Window::Window(Config& config, HINSTANCE hInstance, LPWSTR title, const Window &parent, DWORD styleFlags)
	: myWidth(config.initialWidth), myHeight(config.initialHeight)
{
	if (!config.myRegistered)
	{
		WNDCLASSEXW wndClass = {
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_DBLCLKS,
		.lpfnWndProc = Window::wndProc,
		.cbClsExtra = 0,
		.cbWndExtra = sizeof(LONG_PTR),
		.hInstance = hInstance,
		.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TETESTHOST)),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = config.background == Config::Background::BlackBrush ? (HBRUSH)GetStockObject(BLACK_BRUSH) : (HBRUSH)(COLOR_WINDOW),
		.lpszMenuName = config.menuName,
		.lpszClassName = config.className,
		.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL))
		};

		if (!RegisterClassExW(&wndClass))
		{
			DWORD dwError = GetLastError();
			if (dwError != ERROR_CLASS_ALREADY_EXISTS)
			{
				throw "RegisterClassEx failed";
			}
		}
		config.myRegistered = true;
	}
	RECT rc;
	SetRect(&rc, 0, 0, config.initialWidth, config.initialHeight);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	myWindow = CreateWindowW(config.className,
		title,
		WS_OVERLAPPEDWINDOW | styleFlags,
		CW_USEDEFAULT, CW_USEDEFAULT,
		(rc.right - rc.left), (rc.bottom - rc.top),
		parent.myWindow,
		nullptr,
		hInstance,
		this);
}

Window::~Window()
{
	if (myWindow)
	{
		HMENU menu = GetMenu(myWindow);
		if (menu)
		{
			DestroyMenu(menu);
		}
		DestroyWindow(myWindow);
	}
}

void Window::open(int nCmdShow)
{
	if (myWindow)
	{
		ShowWindow(myWindow, SW_SHOW);
		UpdateWindow(myWindow);
	}	
}

void Window::close() const
{
	CloseWindow(myWindow);
}

void Window::setTimer(UINT_PTR timerID, UINT msec)
{
	myTimers.emplace(timerID, Timer(myWindow, timerID, msec));
}

void Window::cancelTimer(UINT_PTR timerID)
{
	myTimers.erase(timerID);
}

HWND Window::getHWND() const
{
	return myWindow;
}

void Window::setTitle(const std::string& title) const
{
	std::wstring t = ConvertToWide(title);
	SetWindowText(myWindow, t.c_str());
}

unsigned int Window::width() const
{
	return myWidth;
}

unsigned int Window::height() const
{
	return myHeight;
}

bool Window::command(int wmId)
{
	return false;
}

void Window::destroy()
{
	HMENU menu = GetMenu(myWindow);
	if (menu)
	{
		DestroyMenu(menu);
	}
	myWindow = nullptr;
}

void Window::resize(unsigned int w, unsigned int h)
{
	myWidth = w;
	myHeight = h;
}

void Window::timer(UINT_PTR timerID)
{

}

bool Window::paint()
{
	return false;
}

LRESULT CALLBACK
Window::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

		Window* window = (Window*)pcs->lpCreateParams;

		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		return 0;
	}
	case WM_COMMAND:
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		int wmId = LOWORD(wParam);
		if (window->command(wmId))
		{
			return 0;
		}
		break;
	}
	case WM_CLOSE:
	{
		// DefWindowProc will destroy it
		break;
	}
	case WM_DESTROY:
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		window->destroy();
		return 0;
	}
	case WM_SIZE:
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		window->resize(width, height);
		return 0;
	}
	case WM_TIMER:
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		window->timer(wParam);
		return 0;
	}
	case WM_PAINT:
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (window->paint())
		{
			return 0;
		}
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Window::Timer::Timer()
	: myWindow(nullptr), myTimerID(0)
{
}

Window::Timer::Timer(HWND w, UINT_PTR t, UINT m)
	: myWindow(w), myTimerID(SetTimer(w, t, m, nullptr))
{
	if (myTimerID == 0)
	{
		throw std::runtime_error("error creating timer");
	}
}

Window::Timer::~Timer()
{
	if (myWindow && myTimerID)
	{
		KillTimer(myWindow, myTimerID);
	}
}

Window::Timer::Timer(Timer&& o) noexcept
	: myWindow(o.myWindow), myTimerID(o.myTimerID)
{
	o.myTimerID = 0;
	o.myWindow = nullptr;
}

Window::Timer& Window::Timer::operator=(Timer&& o) noexcept
{
	if (&o != this)
	{
		if (myWindow && myTimerID)
		{
			KillTimer(myWindow, myTimerID);
		}
		myWindow = o.myWindow;
		myTimerID = o.myTimerID;
		o.myWindow = nullptr;
		o.myTimerID = 0;
	}
	return *this;
}

Window::Config::Config(const LPWSTR c, const LPWSTR m, Background bg, int iw, int ih)
	: className(c), menuName(m), background(bg), initialWidth(iw), initialHeight(ih)
{
	
}
