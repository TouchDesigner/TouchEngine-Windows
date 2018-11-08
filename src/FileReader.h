#pragma once

#include <string>
#include <vector>

class FileReader
{
public:
    FileReader(const std::wstring &path);
    ~FileReader();
    FileReader(const FileReader &o) = delete;
    FileReader &operator=(const FileReader &o) = delete;
    FileReader(FileReader &&o);
    FileReader &operator=(FileReader &&o);
    bool read(std::vector<unsigned char> &destination);
private:
    HANDLE myFile;
};

