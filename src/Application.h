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

#pragma once
#include <optional>
#include "Window.h"
#include "DocumentWindow.h"
#include "Document.h"

class Renderer;

class Application
{
public:
	Application(HINSTANCE hInstance);
	bool openWindow(int nCmdShow);
	HINSTANCE getInstance() const;
	void about();
	void exit();
	void openDocument(Graphics api);
	const Window& getWindow() const;
	void documentWindowDidClose();
private:
	class AppWindow : public Window {
	public:
		AppWindow(HINSTANCE instance, Application &app);
	protected:
		virtual bool command(int wmId) override;
		virtual bool paint() override;
		virtual void destroy() override;
	private:
		static constexpr int InitialWindowWidth = 600;
		static constexpr int InitialWindowHeight = 320;
		static Config	AppWindowConfig;
		Application&	myApp;
	};
	static INT_PTR CALLBACK dialogProc(HWND, UINT, WPARAM, LPARAM);
	HINSTANCE				myInstance;
	AppWindow				myWindow;
	std::optional<Document>	myDocument;
};

