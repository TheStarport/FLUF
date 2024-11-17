#pragma once
#include <minidumpapiset.h>

class FlufCrashWalkerConfig
{
        std::string_view GetSaveLocation();

    public:
        int miniDumpFlags = MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithModuleHeaders;
        bool applyRestartFlPatch = true;

        bool Save();
        void Load();
};
