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
        float shieldPowerUsage;
};

struct ExtraMunitionData
{};

class Retold final : public FlufModule, public ImGuiModule
{
        std::shared_ptr<FlufUi> flufUi = nullptr;
        std::shared_ptr<EquipmentDealerWindow> equipmentDealerWindow = nullptr;
        DWORD contentDll = 0;
        inline static Retold* instance = nullptr;
        ContentStory* contentStory = nullptr;

        std::unordered_map<EquipmentId, ExtraWeaponData> extraWeaponData;
        std::unordered_map<EquipmentId, ExtraMunitionData> extraMunitionData;

        bool autoTurretsEnabled = true;

        // Hooks

        static FireResult __thiscall GunCanFireDetour(CEGun* gun, Vector& target);
        static void __thiscall LauncherConsumeFireResourcesDetour(CELauncher* launcher);
        static ContentStory* __thiscall ContentStoryCreateDetour(ContentStory* story, void* contentInstance, DWORD* payload);

        void HookContentDll();

        void Render() override;

        void OnGameLoad() override;
        void OnServerStart(const SStartupInfo&) override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnDllUnloaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnFixedUpdate(const double delta) override;
        bool OnKeyToggleAutoTurrets(KeyState state);

        // INI Reading
        DWORD OnSystemIniOpen(INI_Reader& iniReader, const char* file, bool unk);
        static void SystemIniOpenNaked();
        void SetupHooks();
        void ReadUniverseIni();
        void ReadFreelancerIni();
        void ReadEquipmentIni(std::string file);

        std::unordered_map<std::string, std::string> systemFileOverrides;

    public:
        static constexpr std::string_view moduleName = "Retold";

        Retold();
        ~Retold() override;
        std::string_view GetModuleName() override;
};
