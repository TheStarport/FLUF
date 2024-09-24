#include <Windows.h>

#include "FLUF.UI.hpp"

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}