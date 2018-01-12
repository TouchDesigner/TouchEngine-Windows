#include "stdafx.h"
#include "DocumentManager.h"


DocumentManager & DocumentManager::sharedManager()
{
    static DocumentManager manager;
    return manager;
}

bool DocumentManager::open(const std::wstring & path, HWND parent)
{
    for each (auto doc in myDocuments)
    {
        if (doc.second->getPath() == path)
        {
            // already open
            // TODO: order to raise to front
            return true;
        }
    }
    std::shared_ptr<DocumentWindow> opened(std::make_shared<DocumentWindow>(path));
    opened->openWindow(parent);
    myDocuments[opened->getWindow()] = opened;
    return true;
}

std::shared_ptr<DocumentWindow> DocumentManager::lookup(HWND window) const
{
    if (myDocuments.find(window) != myDocuments.end())
    {
        return (*myDocuments.find(window)).second;
    }
    else
    {
        return std::shared_ptr<DocumentWindow>();
    }
}

void DocumentManager::didClose(HWND window)
{
    myDocuments.erase(window);
}

DocumentManager::DocumentManager()
{
}


DocumentManager::~DocumentManager()
{
}
