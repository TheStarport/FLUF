#pragma once
#include "RmlUi/Core/FileInterface.h"

class FileInterface final : public Rml::FileInterface
{
        enum class HandleType
        {
            Web,
            File
        };

        struct VirtualFileHandle
        {
                HandleType type;
                void* handle;

                explicit VirtualFileHandle(FILE* file);
                ~VirtualFileHandle();
        };

    public:
        Rml::FileHandle Open(const Rml::String& path) override;
        void Close(Rml::FileHandle file) override;
        size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
        bool Seek(Rml::FileHandle file, long offset, int origin) override;
        size_t Tell(Rml::FileHandle file) override;

        FileInterface();
        ~FileInterface() override;
};
