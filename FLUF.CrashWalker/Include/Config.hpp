#pragma once
#include <minidumpapiset.h>

class Config
{
        std::string_view GetSaveLocation();

    public:
        int miniDumpFlags = MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithModuleHeaders;
        bool applyRestartFlPatch = true;
        bool useOnlySingleDumpFile = true;

        bool Save();
        void Load();
};
