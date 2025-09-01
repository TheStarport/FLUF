#include "PCH.hpp"

#include "ScreenControl.hpp"
#include <chrono>
#include "Utils/TimeUtils.hpp"
#include "Fluf.hpp"

void ScreenControl::SetBackGroundRunPatch(bool bInWindowedMode)
{
    if (bInWindowedMode)
    {
        BYTE patch1[] = { 0xBA, 0x01, 0x00, 0x00, 0x00, 0x90 };
        MemUtils::WriteProcMem(freelancerExe + 0x1B264C, &patch1, 6);
        BYTE patch2[] = { 0x74 };
        MemUtils::WriteProcMem(freelancerExe + 0x1B2665, &patch2, 1);
    }
    else
    {
        BYTE patch1[] = { 0x83, 0xFF, 0x01, 0x0F, 0x94, 0xC2 };
        MemUtils::WriteProcMem(freelancerExe + 0x1B264C, &patch1, 6);
        BYTE patch2[] = { 0xEB };
        MemUtils::WriteProcMem(freelancerExe + 0x1B2665, &patch2, 1);
    }
}

int __stdcall ScreenControl::AltEnter(int a1)
{
    static int64 lastRunTime = 0;

    if (lastRunTime + 500 > TimeUtils::UnixTime())
    {
        return 1;
    }

    lastRunTime = TimeUtils::UnixTime();
    BYTE buf[0x40];
    memcpy(buf, (DWORD*)(freelancerExe + 0x279BC0), 0x40);
    buf[37] ^= 1;
    SetBackGroundRunPatch(buf[37] ? false : true);
    if (buf[37])
    {
        Fluf::Log(LogLevel::Trace, "Screen Control: Switching to Fullscreen");
    }
    else
    {
        Fluf::Log(LogLevel::Trace, "Screen Control: Switching to Windowed");
    }


    using SetupScreenFunc = int(__cdecl*)(HWND hWnd, BYTE* buf, DWORD* dunno);
    static SetupScreenFunc SetupScreen = (SetupScreenFunc)(freelancerExe + 0x249C0);

    SetupScreen(*FlufUi::mainFreelancerWindow, buf, (DWORD*)(freelancerExe + 0x279BC0));

    return 1;
}

bool GetBoolSetting(const std::string& scSettingName)
{
    std::istringstream args(GetCommandLineA());
    int argc = 0;
    std::string s;
    while (std::getline(args, s, ' '))
    {
        if (s.find(scSettingName) == 0)
        {
            return true;
        }
    }
    return false;
}

void ScreenControl::Patch()
{
    freelancerExe = reinterpret_cast<DWORD>(GetModuleHandleA("freelancer.exe"));

    byte patch1[] = { 51 };
    MemUtils::WriteProcMem(freelancerExe + 0x1b2497, &patch1, 1);
    FARPROC patch2 = (FARPROC)ScreenControl::AltEnter;
    MemUtils::WriteProcMem(freelancerExe + 0x1b254c, &patch2, 4);
    
    SetBackGroundRunPatch(GetBoolSetting("-w"));
}

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);

    if (reason == DLL_PROCESS_ATTACH)
    {
        ScreenControl::Patch();
    }
    return TRUE;
}

ScreenControl::ScreenControl() {};

std::string_view ScreenControl::GetModuleName() { return moduleName; }

SETUP_MODULE(ScreenControl);
