#pragma once

#include "FlufModule.hpp"

#include "RetoldHooks.hpp"

#include <KeyManager.hpp>
#include <memory>
#include <ImGui/ImGuiModule.hpp>

class EquipmentDealerWindow;
class FlufUi;

struct ExtraWeaponData
{
        float shieldPowerUsage = 0.f;
};

struct ExtraMunitionData
{
        float equipmentMultiplier = 0.f;
        float shieldRechargeReduction = 0.f;
        float hullDot = 0.f;
        float hullVulnerability = 0.f;
};

struct ExtraShipData
{
        float hullDotMax = 0.f;
        std::list<std::pair<float, Id>> hullVulnerabilityFuses;
        uint shieldOfflineFuse = 0;
};

struct ExtraShieldData
{
        float shieldStrength = 0.f;
        float offlineRegenerationRate = 0.f;
};

struct ShipDotData
{
        float timeLeft = 0.f;
        float damageToApply = 0.f;
        ushort targetHardpoint = 0;
};

struct ShipHullVulnerability
{
        float duration = 0.f;
        float modifier = 0.f;
        Id fuseId = 0;
        ushort hardPoint = 0;
};

class Retold final : public FlufModule, public ImGuiModule
{
        std::shared_ptr<FlufUi> flufUi = nullptr;
        std::shared_ptr<EquipmentDealerWindow> equipmentDealerWindow = nullptr;
        DWORD contentDll = 0;
        inline static Retold* instance = nullptr;
        ContentStory* contentStory = nullptr;

        std::unordered_map<EquipmentId, ExtraWeaponData> extraWeaponData;
        std::unordered_map<EquipmentId, ExtraMunitionData> extraMunitionData;
        std::unordered_map<EquipmentId, ExtraShipData> extraShipData;
        std::unordered_map<EquipmentId, ExtraShieldData> extraShieldData;
        float equipmentMultipliersToApply;
        std::unordered_map<ShipId, std::list<std::pair<float, float>>> shipShieldRechargeDebuffs;
        std::unordered_map<ShipId, std::list<ShipHullVulnerability>> shipHullVulnerabilities;
        std::unordered_map<ShipId, std::list<ShipDotData>> shipDots;

        float hullVulnerabilityDuration = 5.f;
        float hullVulnerabilityMax = 1.5f;
        std::list<std::pair<float, Id>> hullVulnerabilityFuses;
        float hullDotDuration = 5.f;
        float hullDotMax = 1000.f;
        float shieldRechargeReductionDuration = 5.f;
        float shieldRechargeReductionMax = 0.f;

        bool autoTurretsEnabled = true;

        // Hooks

        using IniReaderOpenDetourType = bool(__thiscall*)(INI_Reader* ini, const char* path, bool unk);
        FunctionDetour<IniReaderOpenDetourType> iniReaderOpenDetour{ reinterpret_cast<IniReaderOpenDetourType>(0x630F9B0) };
        static bool __thiscall IniReaderOpenDetour(INI_Reader* ini, const char* path, bool unk);
        static FireResult __thiscall GunCanFireDetour(CEGun* gun, Vector& target);
        static void __thiscall LauncherConsumeFireResourcesDetour(CELauncher* launcher);
        static ContentStory* __thiscall ContentStoryCreateDetour(ContentStory* story, void* contentInstance, DWORD* payload);
        static void __thiscall ShieldSetHealthDetour(CEShield* shield, float hitPts);
        static void __fastcall ShieldRegenerationPatch(CEShieldGenerator* generator, CEShield* shield, float hitPts);
        static void ShieldRegenerationPatchNaked();

        // Weapons
        void ApplyShipDotStacks(Ship* ship, MunitionImpactData* impact, const ExtraMunitionData& munitionData);
        void ApplyShipVulnerabilityStacks(Ship* ship, MunitionImpactData* impact, const ExtraMunitionData& munitionData);
        void ApplyShieldReductionStacks(Ship* ship, MunitionImpactData* impact, const ExtraMunitionData& munitionData);
        void ProcessShipDotStacks(float delta);
        void RemoveShieldReductionStacks(float delta);
        void RemoveShipVulnerabilityStacks(float delta);

        void HookContentDll();

        void Render() override;

        void OnGameLoad() override;
        void OnServerStart(const SStartupInfo&) override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnDllUnloaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnFixedUpdate(float delta, bool gamePaused) override;
        bool OnKeyToggleAutoTurrets(KeyState state);
        void BeforeShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId) override;
        void BeforeShipMunitionHit(Ship* ship, MunitionImpactData* impact, DamageList* dmgList) override;
        void BeforeShipMunitionHitAfter(Ship* ship, MunitionImpactData* impact, DamageList* dmgList) override;
        void BeforeShipEquipDmg(Ship* ship, CAttachedEquip* equip, float& damage, DamageList* dmgList) override;
        void BeforeShipColGrpDmg(Ship*, CArchGroup* colGrp, float& incDmg, DamageList* dmg) override;
        void BeforeShipHullDamage(Ship* ship, float& damage, DamageList* dmgList) override;
        bool BeforeShipUseItem(Ship* ship, ushort sId, uint count, ClientId clientId) override;

        // INI Reading
        void SetupHooks();
        void ReadUniverseIni();
        void ReadFreelancerIni();
        void ReadEquipmentIni(const std::string& file);
        void ReadShipArchFile(const std::string& file);
        void ReadConstantsIni(const std::string& file);

        std::unordered_map<std::string, std::string> systemFileOverrides;

    public:
        static constexpr std::string_view moduleName = "Retold";

        Retold();
        ~Retold() override;
        std::string_view GetModuleName() override;
};
