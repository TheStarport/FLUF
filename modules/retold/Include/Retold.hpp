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
};

struct ExtraShieldData
{
        float shieldStrength = 0.f;
        float offlineRegenerationRate = 0.f;
        uint shieldOfflineFuse = 0;
};

struct ShipDotData
{
        float timeLeft = 0.f;
        float damageToApply = 0.f;
        ushort targetSId = 0;
        uint inflicterId = 0;
};

struct ShipHealingData
{
        float timeLeft = 0.f;
        bool isShield = false;
};

struct ShipHullVulnerability
{
        float duration = 0.f;
        float modifier = 0.f;
        Id fuseId = 0;
        ushort hardPoint = 0;
};

struct CliLauncher
{
        virtual void dunno0();
        virtual void dunno4();
        virtual void dunno8();
        virtual bool fire(const Vector& pos);
        virtual bool fireForward();

        CELauncher* launcher;
        Ship* owner;

        static constexpr auto PlayFireSound = reinterpret_cast<int(__thiscall*)(CliLauncher*, const Vector& pos, void* unused)>(0x52CED0);
};

constexpr uint MAX_COUNT_SHIELD_EFFECT = 100;
struct ParticleEffect
{
        virtual void StartEffect(struct ParticleDefinition*);
        virtual void StopEffect();
        uint data[0x20];
};
struct CustomShieldHitArray
{
        uint counter = -1;
        std::array<ParticleEffect*, MAX_COUNT_SHIELD_EFFECT> shieldHitArray;

        ~CustomShieldHitArray()
        {
            for (ParticleEffect* ptr : shieldHitArray)
            {
                if (!ptr)
                {
                    continue;
                }

                static auto AleCleanup1 = reinterpret_cast<void(__thiscall*)(void*)>(0x4F8110);
                static auto AleCleanup2 = reinterpret_cast<void(__thiscall*)(void*)>(0x4F7A90);
                AleCleanup1(ptr);
                ptr->StopEffect();
                AleCleanup2(ptr);
            }
        }
};

class Retold final : public FlufModule, public ImGuiModule
{
        float defaultMuzzleCone;

        std::shared_ptr<FlufUi> flufUi = nullptr;
        std::shared_ptr<EquipmentDealerWindow> equipmentDealerWindow = nullptr;
        DWORD contentDll = 0;
        DWORD serverDll = 0;
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
        std::unordered_map<ShipId, std::list<ShipHealingData>> shipHealing;

        inline static std::unordered_map<uint, CustomShieldHitArray> objectShieldHitEffectMap;

        float hullVulnerabilityDuration = 5.f;
        float hullVulnerabilityMax = 1.5f;
        std::list<std::pair<float, Id>> hullVulnerabilityFuses;
        float hullDotDuration = 5.f;
        float hullDotMax = 1000.f;
        float shieldRechargeReductionDuration = 5.f;
        float shieldRechargeReductionMax = 0.f;

        bool autoTurretsEnabled = true;

        float nanobotHealingPerSecond = 0.12f;
        float nanobotHealingDuration = 10.f;
        float batteryHealingPerSecond = 0.1f;
        float batteryHealingDuration = 8.0f;

        // Hooks

        using IniReaderOpenDetourType = bool(__thiscall*)(INI_Reader* ini, const char* path, bool unk);
        FunctionDetour<IniReaderOpenDetourType> iniReaderOpenDetour{ reinterpret_cast<IniReaderOpenDetourType>(0x630F9B0) };
        static bool __thiscall IniReaderOpenDetour(INI_Reader* ini, const char* path, bool unk);
        static FireResult CanGunFire(const CEGun* gun, const Vector& target);
        static FireResult __thiscall GunCanFireDetour(CEGun* gun, Vector& target);
        static void __thiscall LauncherConsumeFireResourcesDetour(CELauncher* launcher);
        static ContentStory* __thiscall ContentStoryCreateDetour(ContentStory* story, void* contentInstance, DWORD* payload);
        static void __thiscall ShieldSetHealthDetour(CEShield* shield, float hitPts);
        static void __fastcall ShieldRegenerationPatch(const CEShieldGenerator* generator, CEShield* shield, float hitPts);
        static void ShieldRegenerationPatchNaked();
        static bool __thiscall ShieldSetActiveDetour(CEShield* shield, bool active);

        // Weapons

        void ApplyDotStacks(EqObj* obj, MunitionImpactData* impact, const ExtraMunitionData& munitionData);
        void ApplyVulnerabilityStacks(EqObj* obj, MunitionImpactData* impact, const ExtraMunitionData& munitionData);
        void ApplyShieldReductionStacks(EqObj* obj, MunitionImpactData* impact, const ExtraMunitionData& munitionData);
        void ProcessDotStacks(float delta);
        void ProcessHealingStacks(float delta);
        void RemoveShieldReductionStacks(float delta);
        void RemoveVulnerabilityStacks(float delta);
        void ProcessEqObjectDamage(const EqObj* ship, float& damage);
        void BeforeComponentDamage(float& damage);
        void BeforeEqObjectMunitionHit(EqObj* obj, MunitionImpactData* impact);

        // Autoturrets

        std::list<EqObj*> autoTurretTargets;
        std::list<CliLauncher*> autoTurrets;
        void ProcessAutoTurrets(float delta);

        void HookContentDll();
        void HookServerDll();

        void Render() override;

        void OnGameLoad() override;
        void OnServerStart(const SStartupInfo&) override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnDllUnloaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnFixedUpdate(float delta, bool gamePaused) override;
        bool OnKeyToggleAutoTurrets(KeyState state);
        void BeforeShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId) override;
        void BeforeSolarDestroy(Solar* ship, DestroyType destroyType, Id killerId) override;

        void BeforeShipMunitionHit(Ship* ship, MunitionImpactData* impact, DamageList* dmgList) override;
        void BeforeShipEquipDmg(Ship* ship, CAttachedEquip* equip, float& damage, DamageList* dmgList) override;
        void BeforeShipColGrpDmg(Ship*, CArchGroup* colGrp, float& incDmg, DamageList* dmg) override;
        void BeforeShipHullDamage(Ship* ship, float& damage, DamageList* dmgList) override;
        void BeforeSolarHullDamage(Solar* ship, float& damage, DamageList* dmgList) override;
        void BeforeSolarMunitionHit(Solar* solar, MunitionImpactData* impact, DamageList* dmgList) override;
        void BeforeSolarColGrpDmg(Solar* solar, CArchGroup* colGrp, float& damage, DamageList* dmg) override;
        void BeforeSolarEquipDmg(Solar* solar, CAttachedEquip* equip, float& damage, DamageList* dmgList) override;

        bool BeforeShipUseItem(Ship* ship, ushort sId, uint count, ClientId clientId) override;
        bool BeforeBaseEnter(uint baseId, uint client) override;

        inline static void* __stdcall GetShieldHitEffectArray(uint id);

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
