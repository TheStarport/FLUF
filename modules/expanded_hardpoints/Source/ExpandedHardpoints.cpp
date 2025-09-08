/*
 * Originally written by Jason Hood, 2010
 * Ported to FLUF by Josbyte, 2025
 */

#include "PCH.hpp"

#include "ExpandedHardpoints.hpp"
#include "Fluf.hpp"
#include "Utils/MemUtils.hpp"
#include <Exceptions.hpp>

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

ExpandedHardpoints::ExpandedHardpoints()
{
    if (!Fluf::IsRunningOnClient())
    {
        LoadHardpointData();
        ServerPatch(GetModuleHandleA("server.dll"));
    }
}

std::string_view ExpandedHardpoints::GetModuleName() { return moduleName; }

void ExpandedHardpoints::OnGameLoad()
{
    Fluf::Info("Expanded Hardpoints: Loading hooks and game data");

    SinglePlayerPatch();

    LoadHardpointData();

    ServerPatch(GetModuleHandleA("server.dll"));
}

void ExpandedHardpoints::OnDllLoaded(std::string_view dllName, HMODULE dllPtr)
{
    if (dllName == "Server.dll") {
        ServerPatch(dllPtr);
    }
}


SETUP_MODULE(ExpandedHardpoints);
