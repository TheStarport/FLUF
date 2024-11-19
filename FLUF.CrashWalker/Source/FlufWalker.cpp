#include "PCH.hpp"

#include "FlufWalker.hpp"

#include "FLUF/Include/Fluf.hpp"

void FlufWalker::OnCallstackEntry(const CallstackEntryType eType, CallstackEntry& entry)
{
#define StrCpy(x, y, z)            \
    if ((y) <= 0)                  \
        return;                    \
    strncpy_s(x, y, z, _TRUNCATE); \
    (x)[(y)-1] = 0;

    if (eType == lastEntry || entry.offset != 0)
    {
        return;
    }

    if (entry.name[0] == 0)
    {
        StrCpy(entry.name, STACKWALK_MAX_NAMELEN, "(function-name not available)");
    }

    if (entry.undName[0] != 0)
    {
        StrCpy(entry.name, STACKWALK_MAX_NAMELEN, entry.undName);
    }

    if (entry.undFullName[0] != 0)
    {
        StrCpy(entry.name, STACKWALK_MAX_NAMELEN, entry.undFullName);
    }

    if (entry.lineFileName[0] == 0)
    {
        StrCpy(entry.lineFileName, STACKWALK_MAX_NAMELEN, "(filename not available)");

        if (entry.moduleName[0] == 0)
        {
            StrCpy(entry.moduleName, STACKWALK_MAX_NAMELEN, "(module-name not available)");
        }

        outputBuffer = std::format("{} ({}): {}: {}\n", static_cast<DWORD>(entry.offset), entry.moduleName, entry.lineFileName, entry.name);
    }
    else
    {
        outputBuffer = std::format("{} ({}): {}\n", entry.lineFileName, entry.lineNumber, entry.name);
    }

#undef StrCpy
    Fluf::Log(LogLevel::Error, outputBuffer);
}

FlufWalker::FlufWalker() = default;
