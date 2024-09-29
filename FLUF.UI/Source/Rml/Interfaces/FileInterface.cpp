#include "PCH.hpp"

#include <regex>

#include "Rml/Interfaces/FileInterface.hpp"

FileInterface::VirtualFileHandle::VirtualFileHandle(FILE* file)
{
    type = HandleType::File;
    handle = file;
}

FileInterface::VirtualFileHandle::~VirtualFileHandle()
{
    if (type == HandleType::File)
    {
        fclose(static_cast<FILE*>(handle));
    }
}

Rml::FileHandle FileInterface::Open(const Rml::String& path)
{
    static const std::regex reg(R"((^\w+(:|\|)|^)\/\/)");
    const std::string rawPath = std::regex_replace(path, reg, "");

    if (path.starts_with("local://") || path.starts_with("local|//"))
    {
        const auto filePath = std::filesystem::current_path().append("../DATA/").append(rawPath);
        auto* file = fopen(filePath.string().c_str(), "rb");
        if (!file)
        {
            return reinterpret_cast<Rml::FileHandle>(nullptr);
        }

        // ReSharper disable twice CppDFAMemoryLeak
        auto* handle = new VirtualFileHandle(file);
        return reinterpret_cast<Rml::FileHandle>(handle);
    }
    if (path.starts_with("file://") || path.starts_with("file|//"))
    {
        auto* file = fopen(rawPath.c_str(), "rb");
        if (!file)
        {
            return reinterpret_cast<Rml::FileHandle>(nullptr);
        }

        // ReSharper disable twice CppDFAMemoryLeak
        auto* handle = new VirtualFileHandle(file);
        return reinterpret_cast<Rml::FileHandle>(handle);
    }

    if (path.starts_with("http"))
    {
        // TODO: handle HTTP/HTTPS via cURL
    }

    return reinterpret_cast<Rml::FileHandle>(nullptr);
}

void FileInterface::Close(const Rml::FileHandle file)
{
    const auto* handle = reinterpret_cast<VirtualFileHandle*>(file);
    delete handle;
}

size_t FileInterface::Read(void* buffer, const size_t size, const Rml::FileHandle file)
{
    switch (const auto* item = reinterpret_cast<VirtualFileHandle*>(file); item->type)
    {
        case HandleType::Web:
            {
                break;
            }
        case HandleType::File:
            {
                return fread(buffer, 1, size, static_cast<FILE*>(item->handle));
            }
    }

    return 0;
}

bool FileInterface::Seek(const Rml::FileHandle file, const long offset, const int origin)
{
    switch (const auto* item = reinterpret_cast<VirtualFileHandle*>(file); item->type)
    {
        case HandleType::File:
            {
                return fseek(static_cast<FILE*>(item->handle), offset, origin) != -1;
            }
        default: return false;
    }
}

size_t FileInterface::Tell(const Rml::FileHandle file)
{
    switch (const auto* item = reinterpret_cast<VirtualFileHandle*>(file); item->type)
    {
        case HandleType::File:
            {
                return ftell(static_cast<FILE*>(item->handle));
            }

        default: return 0;
    }
}

FileInterface::FileInterface() {}

FileInterface::~FileInterface() {}
