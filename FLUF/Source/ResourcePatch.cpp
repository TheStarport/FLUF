#include "PCH.hpp"

#include "Fluf.hpp"

using LoadResourceDll = bool (*)(const char*);
using ForceLoadResource = DWORD (*)(HINSTANCE);
std::unique_ptr<FunctionDetour<LoadResourceDll>> loadResourceDllDetour;

bool Fluf::ResourceDllLoadDetour(const char* unk)
{
    loadResourceDllDetour->UnDetour();
    const auto res = loadResourceDllDetour->GetOriginalFunc()(unk);

    startingResourceIndex = (*reinterpret_cast<PDWORD>(0x67c404) - *reinterpret_cast<PDWORD>(0x67c400)) << 14;

    const auto loadResourceFunc = reinterpret_cast<ForceLoadResource>(0x57d800);
    loadResourceFunc(thisDll);

    loadResourceDllDetour->Detour(ResourceDllLoadDetour);
    return res;
}
