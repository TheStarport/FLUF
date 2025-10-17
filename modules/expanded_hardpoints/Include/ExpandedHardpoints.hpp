#pragma once

#include <Windows.h>
#include "FlufModule.hpp"
#include "Fluf.hpp"

class ExpandedHardpoints final : public FlufModule
{
        using GetVendorItemActiveState = char*(__fastcall*)(void* vendorList, void* edx, uint itemId, bool a3);
        inline static std::unique_ptr<FunctionDetour<GetVendorItemActiveState>> getVendorActiveState;
        struct SlotData
        {
                uint arch;
                bool mounted;
        };

        static constexpr auto Slots = 5;
        static inline SlotData slot[Slots];
        static inline std::unordered_map<Id, std::unordered_set<Id>> equipIdToSubclassesMap;
        static inline std::unordered_map<Id, std::unordered_map<Id, std::vector<std::string>>> shipIdToSubclassesMap;

        // Called when the game loads
        void OnGameLoad() override;
        void OnServerStart(const SStartupInfo&) override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;

        static void Mount1_Hook();
        static std::optional<std::string_view> GetFreeHardpoint(EquipDescList* equipDescList, const std::vector<std::string>& hardpointsToCheck);

        static std::optional<std::string_view> GetFreeHardpointSP(const std::vector<std::string>& hardpointsToCheck);

        static bool IsValidSubclassItem(uint equipArchId, uint shipArch, bool isCurrentlyEquippable);

        static bool __stdcall Mount2_Hook2(SlotData& good);
        static void Mount2_HookNaked();

        static std::optional<std::string_view> GetFreeHardpointMP(PlayerData* pd, const std::vector<std::string>& hardpointsToCheck);

        static void __fastcall SetHardpointMP(EquipDesc* equipDesc, void* edx, PlayerData* pd, CacheString& hardpoint);

        static void __fastcall SetEquippedMP(EquipDesc* equipDesc, void* edx, bool newState);

        void ServerPatch(HMODULE serverDll);
        void LoadHardpointData();
        static bool __stdcall TransferLock(uint newShipArch, uint goodId);
        static void __fastcall SetEquippedDetour(EquipDesc* equipDesc, void* edx, uint shipArch, bool newState);
        static void __fastcall SetHardpointDetour(EquipDesc* equipDesc, void* edx, uint shipArch, CacheString& hardpoint);
        static void __fastcall SetHardpointDetourSelf(EquipDesc* equipDesc, void* edx, CacheString& hardpoint);
        static void SetHardpointDetourDealerNaked();
        static char* __fastcall RenderVendorItemState(void* vendorList, void* edx, uint itemId, bool a3);
        static bool __stdcall FinalEquipmentCheck(uint equipId);
        static void FinalEquipmentCheckNaked();
        void SinglePlayerPatch();

    public:
        static constexpr std::string_view moduleName = "Expanded Hardpoints";

        ExpandedHardpoints();
        ~ExpandedHardpoints() override = default;

        // Required override from FlufModule
        std::string_view GetModuleName() override;
};
