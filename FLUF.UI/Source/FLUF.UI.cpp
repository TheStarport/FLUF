#include "PCH.hpp"

#include "FLUF.UI.hpp"

#include "Vanilla/HudManager.hpp"

std::unique_ptr<FlufUi> module;

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    if (reason == DLL_PROCESS_ATTACH)
    {
        module = std::make_unique<FlufUi>();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        module.reset();
    }

    return TRUE;
}

FlufUi::FlufUi() { hudManager = std::make_shared<HudManager>(); }
FlufUi::~FlufUi() {}
