#include "stdafx.h"
#include "DocumentManager.h"
#include <ShlObj.h>
#include <PathCch.h>
#include <fstream>

DocumentManager & DocumentManager::sharedManager()
{
    static DocumentManager manager;
    return manager;
}

bool DocumentManager::open(const std::wstring & path, HWND parent, DocumentWindow::Mode mode)
{
    return open(path, parent, true, mode);
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

void DocumentManager::render()
{
    for (auto doc : myDocuments)
    {
        doc.second->render();
    }
}

void DocumentManager::didClose(HWND window)
{
    myDocuments.erase(window);
}

void DocumentManager::storeOpenWindows()
{
    if (myDocuments.size() > 0)
    {
        std::wstring path = getSettingsPath();
        if (!path.empty())
        {
            std::wofstream stream(path, std::ofstream::out | std::ofstream::trunc);
            for each (const auto entry in myDocuments)
            {
                stream << entry.second->geTEath() << std::endl;
            }
        }
    }
}

bool DocumentManager::open(const std::wstring & path, HWND parent, bool update, DocumentWindow::Mode mode)
{
    for each (auto doc in myDocuments)
    {
        if (doc.second->geTEath() == path)
        {
            // already open
            SetActiveWindow(doc.second->getWindow());
            return true;
        }
    }
    std::shared_ptr<DocumentWindow> opened(std::make_shared<DocumentWindow>(path, mode));
    opened->openWindow(parent);
    myDocuments[opened->getWindow()] = opened;
    if (update)
    {
        storeOpenWindows();
    }
    return true;
}

void DocumentManager::restoreOpenWindows(HWND parent)
{
    std::wstring path = getSettingsPath();
    if (!path.empty())
    {
        std::wifstream stream(path);
        for (std::wstring line; std::getline(stream, line); )
        {
            open(line, parent, false, DocumentWindow::Mode::DirectX); // TODO: store/get the mode
        }
    }
}

DocumentManager::DocumentManager()
{
}


DocumentManager::~DocumentManager()
{
    
}

std::wstring DocumentManager::getSettingsPath()
{
    PWSTR path;
    auto result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, &path);
    if (result == S_OK)
    {
        WCHAR buffer[MAX_PATH];
        wcscpy_s(buffer, MAX_PATH, path);
        result = PathCchAppend(buffer, MAX_PATH, L"Derivative\\TDPTestHost");
        if (result == S_OK)
        {
            if (CreateDirectoryW(buffer, nullptr) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
            {
                result = S_FALSE;
            }
        }
        if (result == S_OK)
        {
            result = PathCchAppend(buffer, MAX_PATH, L"OpenFileList.txt");
        }


        CoTaskMemFree(path);

        if (result == S_OK)
        {
            return buffer;
        }
    }
    return std::wstring();
}
