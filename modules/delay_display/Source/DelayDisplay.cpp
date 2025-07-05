#include "PCH.hpp"

#include "DelayDisplay.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include <rfl/msgpack.hpp>

void __cdecl DelayDisplayFunc(ushort& sId, char* buffer, int unknown, int remainingAmmo)
{
    CShip* cship = Fluf::GetPlayerCShip();
    CELauncher* launcher = (CELauncher*)cship->equipManager.FindByID(sId);

    float refireRate = launcher->LauncherArch()->refireDelay;

    float remainingCooldown = refireRate - launcher->refireDelayElapsed;

    if (remainingAmmo == -1)
    {
        if (refireRate < 1.0)
        {
            sprintf(buffer, "\0");
        }
        else
        {
            if (remainingCooldown <= 0)
            {
                sprintf(buffer, "0.0");
            }
            else
            {
                sprintf(buffer, "%.1f", remainingCooldown);
            }
        }
        return;
    }

    if (refireRate < 1.0f)
    {
        sprintf(buffer, "%d", remainingAmmo);
        return;
    }

    if (remainingCooldown <= 0)
    {
        sprintf(buffer, "%d--0.0", remainingAmmo);
    }
    else
    {
        sprintf(buffer, "%d--%.1f", remainingAmmo, remainingCooldown);
    }
}

void __declspec(naked) DelayDisplayNaked()
{
    __asm
    {
		push ecx
		push esi
		call DelayDisplayFunc
		pop esi
		add esp, 0x4
		pop eax
		add esp, 0x4
		add eax, 0xB
		mov ecx, [edi + 0x50]
		jmp eax
    }
}

void Patch()
{
    HMODULE hFreelancer = GetModuleHandleA(0);
    BYTE patch[] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD0 };
    DWORD* iAddr = (DWORD*)((char*)&patch + 1);
    *iAddr = reinterpret_cast<DWORD>((void*)DelayDisplayNaked);
    MemUtils::WriteProcMem(DWORD(hFreelancer) + 0xDDFD5, patch, sizeof(patch));
    BYTE patch2[] = { 0x00 };
    MemUtils::WriteProcMem(DWORD(hFreelancer) + 0xDDFC0, patch2, sizeof(patch2));
}

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);

    if (reason == DLL_PROCESS_ATTACH)
    {
        Patch();
    }
    return TRUE;
}

DelayDisplay::DelayDisplay() {}

std::string_view DelayDisplay::GetModuleName() { return moduleName; }

SETUP_MODULE(DelayDisplay);
