#pragma once

#include <unordered_map>

class Window
{
public:
	struct Config {
		enum class Background {
			ColorWindow,
			BlackBrush
		};
		Config(const LPWSTR c, const LPWSTR m, Background bg, int iw, int ih);
		const LPWSTR className;
		const LPWSTR menuName;
		Background background;
		int initialWidth;
		int initialHeight;
		friend class Window;
	private:
		bool myRegistered = false;
	};
	Window() = default;
	Window(Config &config, const HINSTANCE hInstance, LPWSTR title, const Window &parent, DWORD styleFlags);
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	~Window();
	virtual void open(int nCmdShow = SW_SHOW);
	void close() const;
	void setTimer(UINT_PTR timerID, UINT msec);
	void cancelTimer(UINT_PTR timerID);
	HWND getHWND() const;
	void setTitle(const std::string& title) const;
	unsigned int width() const;
	unsigned int height() const;
protected:
	virtual bool command(int wmId);
	virtual void destroy();
	virtual void resize(unsigned int w, unsigned int h);
	virtual void timer(UINT_PTR timerID);
	virtual bool paint();
private:
	struct Timer{
		Timer();
		Timer(HWND w, UINT_PTR t, UINT m);
		~Timer();
		Timer(const Timer&) = delete;
		Timer& operator=(const Timer&) = delete;
		Timer(Timer&& o) noexcept;
		Timer& operator=(Timer&& o) noexcept;
	private:
		HWND myWindow;
		UINT_PTR myTimerID;
	};
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND myWindow = nullptr;
	unsigned int myWidth;
	unsigned int myHeight;
	std::unordered_map<UINT_PTR, Timer> myTimers;
};

