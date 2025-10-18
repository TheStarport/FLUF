#pragma once

#include "FlufModule.hpp"

#include <FLCore/Common/Damage.hpp>

#define RELOFS(from, to) *(PDWORD)((DWORD)(from)) = (DWORD)(to) - (DWORD)(from) - 4

#define NEWOFS(from, to, prev)                    \
    prev = (DWORD)(from) + *((PDWORD)(from)) + 4; \
    RELOFS(from, to)

struct Config
{
        bool borderlessWindowedMode = false;
        bool keepRunningWhenOutOfFocus = true;
        uint windowWidth;
        uint windowHeight;
};

class Fixes final : public FlufModule
{
        static void DisableCharacterFileEncryption();
        static void AllowNegativeDamage();
        static void PatchAudioCodec();
        static void FixNpcLootCapacity();
        static void AddNewShipClasses(HINSTANCE dll);
        static void EnableMenuScrollingWithMouseWheel();
        static void EnsureShieldOfflineWhileInDeathFuse();
        static void MultiplayerReputationFixes();
        static void EnableDynamicAsteroidExtensions();
        static void EnableDamagePerFire();
        static void PatchSectorLetters();
        static void AllowViewingOfServerInfoForDifferentVersions();
        static void ForceWeaponPrimaryWeaponGroupOnLaunch();
        static void FixSoundsNotBeingPlayed();
        static void AllowUndockingFromNonTargetableObject();

        void OnGameLoad() override;
        void OnServerStart(const SStartupInfo&) override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;

        inline static rfl::Box<Config> config;

    public:
        static constexpr std::string_view configName = "modules/config/fixes.yml";
        static constexpr std::string_view moduleName = "Fixes";

        Fixes();
        ~Fixes() override;
        EXPORT std::string_view GetModuleName() override;
};
