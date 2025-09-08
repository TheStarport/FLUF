#include "PCH.hpp"

#include "ExpandedHardpoints.hpp"
#include "Utils/MemUtils.hpp"

bool overrideEquipState = false;

void __fastcall ExpandedHardpoints::SetHardpointMP(EquipDesc* equipDesc, void* edx, PlayerData* pd, CacheString& hardpoint)
{
    auto itemClassIter = equipIdToSubclassesMap.find(equipDesc->archId);
    if (itemClassIter == equipIdToSubclassesMap.end())
    {
        equipDesc->set_hardpoint(hardpoint);
        return;
    }

    static Id lastProcessedShipId = Id();
    static auto currShipEquipClass = shipIdToSubclassesMap.find(pd->shipArchetype);

    if (currShipEquipClass == shipIdToSubclassesMap.end())
    {
        hardpoint = EquipDesc::CARGO_BAY_HP_NAME;
        overrideEquipState = true;
        equipDesc->set_hardpoint(hardpoint);
        return;
    }

    for (auto& subClass : itemClassIter->second)
    {
        if (auto subclass = currShipEquipClass->second.find(subClass); subclass != currShipEquipClass->second.end())
        {
            if (auto freeHardpoint = GetFreeHardpoint(&pd->equipAndCargo, subclass->second); freeHardpoint.has_value())
            {
                sprintf(hardpoint.value, freeHardpoint.value().data());
                equipDesc->set_hardpoint(hardpoint);
                return;
            }
        }
    }

    hardpoint = EquipDesc::CARGO_BAY_HP_NAME;
    overrideEquipState = true;
    equipDesc->set_hardpoint(hardpoint);
}

__declspec(naked) void SetHardpoint1Naked()
{
    __asm {
        push [esp + 0x4]
        push esi
        call ExpandedHardpoints::SetHardpointMP 
        ret 4
    }
}

void __fastcall ExpandedHardpoints::SetEquippedMP(EquipDesc* equipDesc, void* edx, bool newState)
{
    if (overrideEquipState)
    {
        overrideEquipState = false;
        equipDesc->set_equipped(false);
        return;
    }
    equipDesc->set_equipped(newState);
}

//__declspec(naked) void SetEquipped1Naked()
//{
//    __asm {
//        push [esp + 0x4]
//        call ExpandedHardpoints::SetEquippedDetour
//        ret 4
//    }
//}

void ExpandedHardpoints::ServerPatch(HMODULE serverDll)
{
    BYTE jumpPatch[] = { 0x90, 0xE8 };
    MemUtils::WriteProcMem(DWORD(serverDll) + 0x6F067, jumpPatch, 2);
    MemUtils::PatchCallAddr(serverDll, 0x6F068, SetHardpoint1Naked);

    MemUtils::WriteProcMem(DWORD(serverDll) + 0x6F094, jumpPatch, 2);
    MemUtils::PatchCallAddr(serverDll, 0x6F095, SetEquippedMP);
}
