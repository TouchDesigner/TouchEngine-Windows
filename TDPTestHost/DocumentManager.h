#pragma once

#include <map>
#include <memory>
#include "DocumentWindow.h"

class DocumentManager
{
public:
    static DocumentManager & sharedManager();
    bool open(const std::wstring &path, HWND parent, DocumentWindow::Mode mode);
    std::shared_ptr<DocumentWindow> lookup(HWND window) const;
    void render();
    void didClose(HWND window);
    void storeOpenWindows();
    void restoreOpenWindows(HWND parent);
private:
    DocumentManager();
    ~DocumentManager();
    bool open(const std::wstring &path, HWND parent, bool update, DocumentWindow::Mode mode);
    static std::wstring getSettingsPath();
    std::map<HWND, std::shared_ptr<DocumentWindow>> myDocuments;
};

