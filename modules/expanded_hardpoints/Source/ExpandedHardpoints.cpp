/*
 * Originally written by Jason Hood as eng_class, 2010
 * Rewritten by FLUF by Aingar, 2025
 */

#include "PCH.hpp"

#include "ExpandedHardpoints.hpp"
#include "Fluf.hpp"

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

ExpandedHardpoints::ExpandedHardpoints() = default;

std::string_view ExpandedHardpoints::GetModuleName() { return moduleName; }

void ExpandedHardpoints::OnGameLoad()
{
    Fluf::Info("Expanded Hardpoints: Loading hooks and game data");

    SinglePlayerPatch();
    LoadHardpointData();

    ServerPatch(GetModuleHandleA("server.dll"));
}
void ExpandedHardpoints::OnServerStart(const SStartupInfo& startup_info)
{
    LoadHardpointData();
    ServerPatch(GetModuleHandleA("server.dll"));
}

void ExpandedHardpoints::OnDllLoaded(std::string_view dllName, HMODULE dllPtr)
{
    if (dllName == "Server.dll")
    {
        ServerPatch(dllPtr);
    }
}

SETUP_MODULE(ExpandedHardpoints);
