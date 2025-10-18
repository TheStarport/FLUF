#include "PCH.hpp"

#include "Fixes.hpp"
#include "Fluf.hpp"

#include "Utils/MemUtils.hpp"
#include "Utils/StringUtils.hpp"

const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

HINSTANCE dll;
BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        dll = mod;
    }

    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void Fixes::OnGameLoad()
{
    PatchAudioCodec();
    AllowNegativeDamage();
    FixNpcLootCapacity();
    EnableMenuScrollingWithMouseWheel();
    EnsureShieldOfflineWhileInDeathFuse();
    MultiplayerReputationFixes();
    EnableDynamicAsteroidExtensions();
    EnableDamagePerFire();

    PatchSectorLetters();
    ForceWeaponPrimaryWeaponGroupOnLaunch();
    FixSoundsNotBeingPlayed();

    DisableCharacterFileEncryption();
}

void Fixes::OnServerStart(const SStartupInfo&)
{
    AllowNegativeDamage();
    FixNpcLootCapacity();
    EnsureShieldOfflineWhileInDeathFuse();
    EnableDamagePerFire();

    DisableCharacterFileEncryption();
}

void Fixes::OnDllLoaded(std::string_view dllName, HMODULE dllPtr)
{
    // Repatch server hooks as needed
    if (_strcmpi(dllName.data(), "server.dll") == 0 || _strcmpi(dllName.data(), "content.dll") == 0)
    {
        AllowNegativeDamage();
        EnableDamagePerFire();
        DisableCharacterFileEncryption();
    }
}

Fixes::Fixes()
{
    if (const auto conf = ConfigHelper<Config>::Load(configName, true); conf.has_value())
    {
        config = rfl::Box<Config>::make(*conf);
    }

    if (Fluf::IsRunningOnClient())
    {
        AllowViewingOfServerInfoForDifferentVersions();
        AddNewShipClasses(dll);
    }
}

Fixes::~Fixes() = default;

std::string_view Fixes::GetModuleName() { return moduleName; }

SETUP_MODULE(Fixes);
