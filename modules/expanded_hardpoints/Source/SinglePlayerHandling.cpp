#include "PCH.hpp"

#include "ExpandedHardpoints.hpp"
#include "Utils/MemUtils.hpp"

#define NAKED                  __declspec(naked)

#define ADDR_INFO              ((PDWORD)(0x4767d3 + 1))
#define ADDR_XFER              ((PDWORD)(0x47b008 + 2))
#define ADDR_MOUNT1            ((PDWORD)(0x47c8c3 + 2))
#define ADDR_INTERN            ((PBYTE)(0x47e4c0 + 1))
#define ADDR_MOUNT2            ((PDWORD)(0x47f5aa + 1))
#define ADDR_MOUNT3a           ((PDWORD)(0x47f7e0 + 1))
#define ADDR_MOUNT3b           ((PDWORD)(0x47f7ee + 2))
#define ADDR_DEALER1           ((PDWORD)(0x480892 + 2))
#define ADDR_DEALER2           ((PDWORD)(0x48092b + 2))

#define find_by_archetype_call call dword ptr ds : [0x5c629c]

DWORD dummyz;
#define ProtectX(addr, size) VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &dummyz)

#define RELOFS(from, to)     *(PDWORD)(from) = (DWORD)(to) - (DWORD)(from) - 4

#define REL2ABS(addr)        ((DWORD)(addr) + *((PDWORD)(addr)) + 4)

#define NEWOFS(from, to, prev) \
    prev = REL2ABS(from);      \
    RELOFS(from, to)

#define INDIRECT(from, to, org) \
    org = **(PDWORD*)(from);    \
    *(PDWORD*)(from) = &to

DWORD Mount1_Org, Mount2_Org, Mount3a_Org, Mount3b_Org;
DWORD Dealer1_Org, Dealer2_Org, Xfer_Org, Info_Org;

#define SLOTS 5
ExpandedHardpoints::SlotData slot[SLOTS];

// Create a list of the archetypes in each slot.
NAKED
void Mount1_Hook()
{
    __asm {
		cmp [esp + 0x24], 0 // check if on player inventory, not vendor
		jz skip

		push ebx
		push ecx
		push eax
		push edi

		lea ebx, slot
		mov ecx, eax
		mov eax, edi
		mov edi, 8
		mul edi
		add ebx, eax
		mov [ebx], ecx
		add ebx, 4
		mov ecx, [esp+0x58]
		mov [ebx], ecx

		pop edi
		pop eax
		pop ecx
		pop ebx


	skip:
		jmp	Mount1_Org
		align	16
    }
}

std::optional<std::string_view> ExpandedHardpoints::GetFreeHardpoint(EquipDescList* equipDescList, const std::vector<std::string>& hardpointsToCheck)
{

    std::unordered_set<std::string_view> occupiedHPs;
    for (auto& desc : equipDescList->equip)
    {
        if (!desc.mounted)
        {
            continue;
        }

        occupiedHPs.insert(desc.hardPoint.value);
    }

    occupiedHPs.erase("BAY");

    for (auto& hp : hardpointsToCheck)
    {
        if (!occupiedHPs.count(hp))
        {
            return { hp };
        }
    }

    return {};
}

std::optional<std::string_view> ExpandedHardpoints::GetFreeHardpointSP(const std::vector<std::string>& hardpointsToCheck)
{
    return GetFreeHardpoint(Fluf::GetPlayerEquipDesc(), hardpointsToCheck);
}

bool ExpandedHardpoints::IsValidSubclassItem(uint equipArchId, uint shipArch, bool checkIfCurrentlyEquippable)
{
    auto itemClassIter = equipIdToSubclassesMap.find(equipArchId);
    if (itemClassIter == equipIdToSubclassesMap.end())
    {
        return true;
    }

    static uint lastProcessedShipId = 0;
    static auto currShipEquipClass = shipIdToSubclassesMap.find(shipArch);

    if (lastProcessedShipId != shipArch)
    {
        currShipEquipClass = shipIdToSubclassesMap.find(shipArch);
        lastProcessedShipId = shipArch;
    }

    if (currShipEquipClass == shipIdToSubclassesMap.end())
    {
        return false;
    }

    for (auto& subClass : itemClassIter->second)
    {
        if (!checkIfCurrentlyEquippable)
        {
            if (currShipEquipClass->second.count(subClass))
            {
                return true;
            }
        }
        else if (auto iter = currShipEquipClass->second.find(subClass); iter != currShipEquipClass->second.end())
        {
            if (auto freeHardpoint = GetFreeHardpointSP(iter->second))
            {
                return freeHardpoint.has_value();
            }
        }
    }

    return false;
}

bool __stdcall ExpandedHardpoints::Mount2_Hook2(SlotData& good)
{
    typedef bool (*OrgMount2Func)();
    const static OrgMount2Func call = (OrgMount2Func)(0x4C4810);

    bool orgRetVal = call();
    if (!orgRetVal)
    {
        return false;
    }

    if (good.mounted)
    {
        const GoodInfo* gi = GoodList::find_by_archetype(good.arch);
        if (!gi || gi->price == 0.0f)
        {
            return false;
        }
        return true;
    }

    return IsValidSubclassItem(good.arch, Fluf::GetPlayerShipArchId(), true);
}

uint mount2RetAddr = 0x47F5AF;
NAKED void Mount2_HookNaked()
{
    __asm {
		mov eax, [esp + 0x18]
		add esp, 4
		lea eax, slot[eax*8]
		push eax
		push mount2RetAddr
		jmp ExpandedHardpoints::Mount2_Hook2
    }
}

// Prevent automatic unmount of internal equipment.
NAKED
void Mount3a_Hook()
{
    __asm {
		push[edi + 4]
		find_by_archetype_call
		add	esp, 4
		test	eax, eax
		jz	done
		cmp	dword ptr[eax + 0x58], 0 // price
		jnz	done
		mov	esi, edi // mount this item instead of
		ret //  the selected item
		done :
		jmp	Mount3a_Org
			align	16
    }
}

NAKED
void Mount3b_Hook()
{
    __asm {
		push[edi + 4]
		find_by_archetype_call
		add	esp, 4
		test	eax, eax
		jz	done
		cmp	dword ptr[eax + 0x58], 0 // price
		jnz	done
		mov	esi, edi // mount this item instead of
		ret	4 //  the selected item
		done:
		jmp	Mount3b_Org
			align	16
    }
}

// Prevent selling and transferring.
NAKED
void Dealer1_Hook()
{
    __asm {
		push	ecx
		find_by_archetype_call
		add	esp, 4
		test	eax, eax
		jz	done
		cmp	dword ptr[eax + 0x58], 0 // price
		jnz	done
		pop	eax // return address
		pop	ecx // arg
		add	eax, 0x2f // 4808C7
		push	0
		push	0 // no message
		jmp	eax
		done :
		jmp	Dealer1_Org
			align	16
    }
}

NAKED
void Dealer2_Hook()
{
    __asm {
		push	eax
		find_by_archetype_call
		add	esp, 4
		test	eax, eax
		jz	done
		cmp	dword ptr[eax + 0x58], 0 // price
		jnz	done
		pop	eax // return address
		pop	ecx // arg
		add	eax, 0x2c // 48095D
		push	0
		push	0 // no message
		mov	ecx, esi
		jmp	eax
		done :
		jmp	Dealer2_Org
			align	16
    }
}

bool __stdcall ExpandedHardpoints::TransferLock(uint newShipArchId, uint goodId)
{
    auto gi = GoodList::find_by_archetype(goodId);
    if (!gi || gi->price == 0.0f)
    {
        return false;
    }

    return IsValidSubclassItem(goodId, newShipArchId, false);
}

// Prevent transferring to the new ship.
NAKED
void Xfer_Hook()
{
    __asm {
		push	eax
        mov eax, [esp+0x34]
        push [eax+0x8]
        call ExpandedHardpoints::TransferLock
        test eax, eax
        jnz done
		xor edi, edi
		done :
		jmp	Xfer_Org
			align	16
    }
}

// Prevent base info from displaying it as a commodity to buy.
NAKED
void Info_Hook()
{
    __asm {
		mov	eax, [esp + 0x64]
		cmp	dword ptr[eax + 0x58], 0
		jz	bad
		jmp	Info_Org
		bad :
		fld1
			fchs
			ret	4
    }
}

DWORD Mount1_New = (DWORD)Mount1_Hook;
DWORD Mount3b_New = (DWORD)Mount3b_Hook;
DWORD Dealer1_New = (DWORD)Dealer1_Hook;
DWORD Dealer2_New = (DWORD)Dealer2_Hook;
DWORD Xfer_New = (DWORD)Xfer_Hook;

void __fastcall ExpandedHardpoints::SetEquippedDetour(EquipDesc* equipDesc, void* edx, uint shipArch, bool newState) { equipDesc->set_equipped(newState); }

void __fastcall ExpandedHardpoints::SetHardpointDetour(EquipDesc* equipDesc, void* edx, uint shipArch, CacheString& hardpoint)
{
    auto itemClassIter = equipIdToSubclassesMap.find(equipDesc->archId);
    if (itemClassIter == equipIdToSubclassesMap.end())
    {
        equipDesc->set_hardpoint(hardpoint);
        return;
    }

    static Id lastProcessedShipId = Id();
    static auto currShipEquipClass = shipIdToSubclassesMap.find(shipArch);

    if (lastProcessedShipId != shipArch)
    {
        currShipEquipClass = shipIdToSubclassesMap.find(shipArch);
        lastProcessedShipId = shipArch;
    }

    if (currShipEquipClass == shipIdToSubclassesMap.end())
    {
        hardpoint = EquipDesc::CARGO_BAY_HP_NAME;
        equipDesc->set_hardpoint(hardpoint);
        return;
    }

    for (auto& subClass : itemClassIter->second)
    {
        if (auto subclass = currShipEquipClass->second.find(subClass); subclass != currShipEquipClass->second.end())
        {
            if (auto freeHardpoint = GetFreeHardpointSP(subclass->second); freeHardpoint.has_value())
            {
                sprintf(hardpoint.value, freeHardpoint.value().data());
                equipDesc->set_hardpoint(hardpoint);
                return;
            }
        }
    }

    hardpoint = EquipDesc::CARGO_BAY_HP_NAME;
    equipDesc->set_hardpoint(hardpoint);
}

void __fastcall ExpandedHardpoints::SetHardpointDetourSelf(EquipDesc* equipDesc, void* edx, CacheString& hardpoint)
{
    SetHardpointDetour(equipDesc, edx, Fluf::GetPlayerShipArchId(), hardpoint);
}

uint SetHardpointDealer2RetAddr = 0x4C6760;
__declspec(naked) void ExpandedHardpoints::SetHardpointDetourDealerNaked()
{
    __asm {
        push [esp+0x4]
        push [ebp + 0x18]
        call ExpandedHardpoints::SetHardpointDetour 
        ret 4
    }
}

char* __fastcall ExpandedHardpoints::RenderVendorItemState(void* vendorList, void* edx, uint itemId, bool a3)
{
    getVendorActiveState->UnDetour();
    char* retVal = getVendorActiveState->GetOriginalFunc()(vendorList, edx, itemId, a3);
    getVendorActiveState->Detour(RenderVendorItemState);

    if (!retVal)
    {
        return retVal;
    }

    if (IsValidSubclassItem(itemId, Fluf::GetPlayerShipArchId(), false))
    {
        return retVal;
    }
    else
    {
        return nullptr;
    }
}

bool __stdcall FinalEquipmentCheck(uint equipId) { return ExpandedHardpoints::IsValidSubclassItem(equipId, Fluf::GetPlayerShipArchId(), false); }

uint FinalRetJump = 0x482ED8;
__declspec(naked) void FinalEquipmentCheckNaked()
{
    __asm
    {
        push ebx
        call FinalEquipmentCheck
        test al, al
        jnz cont
        pop edi
        pop esi
        pop ebp
        mov al, 0
        pop ebx
        pop ecx
        ret 4

        cont:
        mov eax, [edi+0x974]
        jmp FinalRetJump
    }
}

void ExpandedHardpoints::SinglePlayerPatch()
{
    ProtectX(ADDR_INFO, 4);
    ProtectX(ADDR_XFER, 4);
    ProtectX(ADDR_MOUNT1, 4);
    ProtectX(ADDR_INTERN, 1);
    ProtectX(ADDR_MOUNT2, 4);
    //ProtectX( ADDR_MOUNT3a, 4 );
    //ProtectX( ADDR_MOUNT3b, 4 );
    ProtectX(ADDR_DEALER1, 4);
    //ProtectX( ADDR_DEALER2, 4 );

    *ADDR_INTERN = 0; // show internal equipment with a price of 0
    INDIRECT(ADDR_MOUNT1, Mount1_New, Mount1_Org);
    NEWOFS(ADDR_MOUNT2, Mount2_HookNaked, Mount2_Org);
    NEWOFS(ADDR_MOUNT3a, Mount3a_Hook, Mount3a_Org);
    INDIRECT(ADDR_MOUNT3b, Mount3b_New, Mount3b_Org);
    INDIRECT(ADDR_DEALER1, Dealer1_New, Dealer1_Org);
    INDIRECT(ADDR_DEALER2, Dealer2_New, Dealer2_Org);
    INDIRECT(ADDR_XFER, Xfer_New, Xfer_Org);
    NEWOFS(ADDR_INFO, Info_Hook, Info_Org);

    BYTE jumpPatch[] = { 0x90, 0xE8 };
    MemUtils::WriteProcMem(DWORD(GetModuleHandleA(nullptr)) + 0x7FA27, jumpPatch, 2);
    MemUtils::PatchCallAddr(GetModuleHandleA(nullptr), 0x7FA28, SetHardpointDetourSelf);

    MemUtils::WriteProcMem(DWORD(GetModuleHandleA(nullptr)) + 0xC675A, jumpPatch, 2);
    MemUtils::PatchCallAddr(GetModuleHandleA(nullptr), 0xC675B, SetHardpointDetourDealerNaked);

    //getVendorActiveState = std::make_unique<FunctionDetour<GetVendorItemActiveState>>(reinterpret_cast<GetVendorItemActiveState>(0x585F20));
    //getVendorActiveState->Detour(RenderVendorItemState);
    
    BYTE jump = 0xE9;
    MemUtils::WriteProcMem(DWORD(GetModuleHandleA(nullptr)) + 0x82ED2, &jump, 1);
    MemUtils::PatchCallAddr(GetModuleHandleA(nullptr), 0x82ED2, FinalEquipmentCheckNaked);
}
