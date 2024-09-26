#include "PCH.hpp"

#include "FLUF.UI.hpp"

#include "Typedefs.hpp"
#include "Vanilla/HudManager.hpp"

std::shared_ptr<FlufUi> module;

// ReSharper disable twice CppUseAuto
st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    if (reason == DLL_PROCESS_ATTACH)
    {
        module = std::make_shared<FlufUi>();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        module.reset();
    }

    return TRUE;
}

std::weak_ptr<FlufUi> FlufUi::Instance() { return module; }
std::weak_ptr<HudManager> FlufUi::GetHudManager() { return hudManager; }

FlufUi::FlufUi() { hudManager = std::make_shared<HudManager>(); }
FlufUi::~FlufUi() { hudManager.reset(); }
