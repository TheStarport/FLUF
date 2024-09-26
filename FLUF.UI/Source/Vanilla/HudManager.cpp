// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include <Utils/Utils.hpp>

#include "Vanilla/AbstractVanillaInterface.hpp"
#include "Vanilla/HudManager.hpp"

void __declspec(naked) HudManager::OnDraw()
{
    static constexpr DWORD retAddr = 0x59E48E;
    __asm
    {
        call dword ptr [edx+0x34]
        push ebx
        call OnDrawInner
        mov edx, [esp+0x14]
        jmp retAddr
    }
}

void __stdcall HudManager::OnDrawInner(FlControl* control)
{
    for (const auto hud : huds)
    {
        hud->OnDraw(control);
    }
}

void __declspec(naked) HudManager::OnUpdate()
{
    static constexpr DWORD retAddr = 0x59E261;
    __asm
    {
        push      ebx
        mov       ebx,ecx
        mov       eax,0x2C
        mov       edx,0x30
        test      [ecx+0x6C], 1
        cmovz     eax,edx
        mov       edx,[ecx]
        call      dword ptr [edx+eax]
        push ebx
        call OnUpdateInner
        pop ebx
        jmp retAddr
    }
}

void __stdcall HudManager::OnUpdateInner(FlControl* control)
{
    for (const auto hud : huds)
    {
        hud->OnUpdate(control);
    }
}

void __declspec(naked) HudManager::OnExecuteButton()
{
    __asm
    {
        pushad
        push ebx
        call OnExecuteButtonInner
        test al, al
        popad

        je  done // If false don't call original execute function
        call dword ptr [edx+0xB4]

        done:
        pop edi
        pop esi
        mov al, 1
        pop ebx
        add esp, 0x48
        ret 4
    }
}

bool __stdcall HudManager::OnExecuteButtonInner(FlButtonControl* control)
{
    for (const auto hud : huds)
    {
        if (!hud->OnExecuteButton(control))
        {
            return false;
        }
    }

    return true;
}

void __declspec(naked) HudManager::OnExecuteText()
{
    __asm {
        pushad
        push esi
        call OnExecuteTextInner
        test al, al
        popad

        je  done
        call dword ptr [ebp+0xA8]

        done:
        pop edi
        pop esi
        pop ebp
        mov al,bl
        pop ebx
        ret 4
    }
}

bool __stdcall HudManager::OnExecuteTextInner(FlTextControl* control)
{
    for (const auto hud : huds)
    {
        if (!hud->OnExecuteText(control))
        {
            return false;
        }
    }

    return true;
}

void __declspec(naked) HudManager::OnExecuteScroll()
{
    __asm {
        test bl, bl
        jz done

        pushad
        push esi
        call OnExecuteScrollInner
        popad

        done:
        pop esi
        pop ebp
        mov al, bl
        pop ebx
        add esp, 0x24
        ret 4
    }
}

void __stdcall HudManager::OnExecuteScrollInner(FlScrollControl* control)
{
    for (const auto hud : huds)
    {
        hud->OnExecuteScroll(control);
    }
}

void __declspec(naked) HudManager::OnExecuteCycle()
{
    __asm {
        pushad

        push esi
        call OnExecuteCycleInner
        popad

        pop edi
        pop esi
        add esp, 0x21
        mov al, 1
        ret 4
    }
}

void __stdcall HudManager::OnExecuteCycleInner(FlCycleControl* control)
{
    for (const auto hud : huds)
    {
        hud->OnExecuteCycle(control);
    }
}

void __declspec(naked) HudManager::OnTerminate()
{
    static constexpr DWORD retAddr = 0x59DC0E;
    __asm {
        jz cont

        pushad
        push edx
        call OnTerminateInner
        popad

        mov al, 1
        ret

        cont:
        jmp retAddr
    }
}

void __stdcall HudManager::OnTerminateInner(FlControl* control)
{
    for (const auto hud : huds)
    {
        hud->OnTerminate(control);
    }
}

HudManager::HudManager()
{
    MemUtils::PatchAssembly(0x59E40B, OnDraw);
    MemUtils::PatchAssembly(0x59E1B8, OnUpdate);
    MemUtils::PatchAssembly(0x55BB26, OnExecuteButton);
    MemUtils::PatchAssembly(0x58AE2A, OnExecuteText);
    MemUtils::PatchAssembly(0x5971B3, OnExecuteScroll);
    MemUtils::PatchAssembly(0x5979BF, OnExecuteCycle);
    MemUtils::PatchAssembly(0x59DC09, OnTerminate);
}

HudManager::~HudManager()
{
    // Game crashes if we don't hit the restore button
    const std::array<byte, 5> restoreTermiante = { 0x74, 0x03, 0xB0, 0x01, 0xC3 };
    MemUtils::WriteProcMem(0x59DC09, restoreTermiante.data(), restoreTermiante.size());
}

bool HudManager::RegisterHud(AbstractVanillaInterface* hud)
{
    if (hud == nullptr)
    {
        return false;
    }

    return huds.emplace(hud).second;
}

bool HudManager::EraseHud(AbstractVanillaInterface* hud) { return huds.erase(hud); }
