#pragma once

#include <map>
#include <memory>
#include "DocumentWindow.h"

class DocumentWindow;

class DocumentManager
{
public:
    static DocumentManager & sharedManager();
    bool open(const std::wstring &path, HWND parent);
    std::shared_ptr<DocumentWindow> lookup(HWND window) const;
    void didClose(HWND window);
private:
    DocumentManager();
    ~DocumentManager();
    std::map<HWND, std::shared_ptr<DocumentWindow>> myDocuments;
};

