#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "Retold.hpp"

#include <FLUF.UI.hpp>

#include <ImGui/ImGuiInterface.hpp>
#include <EquipmentDealerWindow.hpp>
#include <Utils/MemUtils.hpp>

#include <ini.h>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void Retold::SetupHooks()
{
    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    const auto common = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll"));

    iniReaderOpenDetour.Detour(IniReaderOpenDetour);

    RetoldHooks::gunCanFireDetour.Detour(GunCanFireDetour);
    RetoldHooks::consumeFireResourcesDetour.Detour(LauncherConsumeFireResourcesDetour);
    RetoldHooks::shieldSetHealthDetour.Detour(ShieldSetHealthDetour);

    MemUtils::PatchAssembly(common + 0x3CF06, ShieldRegenerationPatchNaked);
}

ContentStory* __thiscall Retold::ContentStoryCreateDetour(ContentStory* story, void* contentInstance, DWORD* payload)
{
    instance->contentStory = story;
    RetoldHooks::contentStoryCreateDetour->UnDetour();
    RetoldHooks::contentStoryCreateDetour->GetOriginalFunc()(story, contentInstance, payload);
    RetoldHooks::contentStoryCreateDetour->Detour(ContentStoryCreateDetour);
    return story;
}

void Retold::HookContentDll()
{
    contentDll = reinterpret_cast<DWORD>(GetModuleHandleA("content.dll"));
    if (!contentDll || RetoldHooks::contentStoryCreateDetour)
    {
        return;
    }

    RetoldHooks::contentStoryCreateDetour = std::make_unique<FunctionDetour<RetoldHooks::CreateContentMessageHandler<ContentStory*>>>(
        reinterpret_cast<RetoldHooks::CreateContentMessageHandler<ContentStory*>>(contentDll + 0x5E10));
    RetoldHooks::contentStoryCreateDetour->Detour(ContentStoryCreateDetour);
}

bool Retold::OnKeyToggleAutoTurrets(const KeyState state)
{
    if (state == KeyState::Released)
    {
        autoTurretsEnabled = !autoTurretsEnabled;
    }
    return true;
}

void Retold::OnGameLoad()
{
    defaultMuzzleCone = MUZZLE_CONE_ANGLE * (std::numbers::pi_v<float> / 180.f);
    HookContentDll();

    auto* km = Fluf::GetKeyManager();
    km->RegisterKey(this, "FLUF_TOGGLE_AUTO_TURRETS", Key::USER_WARP_OVERRIDE, static_cast<KeyFunc>(&Retold::OnKeyToggleAutoTurrets));

    if (flufUi->GetConfig()->uiMode != UiMode::ImGui)
    {
        return;
    }

    auto imgui = flufUi->GetImGuiInterface();
    //imgui->RegisterImGuiModule(this);
    //imgui->RegisterOptionsMenu(this, static_cast<RegisterOptionsFunc>(&Retold::RenderRetoldOptions));

    //equipmentDealerWindow = std::make_unique<EquipmentDealerWindow>(imgui);
}

void Retold::OnServerStart(const SStartupInfo& startup_info)
{
    defaultMuzzleCone = MUZZLE_CONE_ANGLE * (std::numbers::pi_v<float> / 180.f);

    HookContentDll();
}

void Retold::Render() { equipmentDealerWindow->Render(); }

void Retold::OnDllLoaded(std::string_view dllName, HMODULE dllPtr)
{
    if (dllName.ends_with("content.dll") || dllName.ends_with("Content.dll"))
    {
        HookContentDll();
    }
}

void Retold::OnDllUnloaded(std::string_view dllName, HMODULE dllPtr)
{
    if (dllName.ends_with("content.dll") || dllName.ends_with("Content.dll"))
    {
        contentDll = 0;
        RetoldHooks::contentStoryCreateDetour = nullptr;
    }
}

void Retold::OnFixedUpdate(const float delta, bool gamePaused)
{
    if (gamePaused)
    {
        return;
    }

    ProcessShipDotStacks(delta);
    RemoveShieldReductionStacks(delta);
    RemoveShipVulnerabilityStacks(delta);

    ProcessAutoTurrets(delta);
}

Retold::Retold()
{
    instance = this;

    if (!Fluf::IsRunningOnClient())
    {
        return;
    }

    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        throw ModuleLoadException("RaidUi was loaded, but FLUF UI was not loaded.");
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    flufUi = module;

    if (module->GetConfig()->uiMode != UiMode::ImGui)
    {
        throw ModuleLoadException("RaidUi was loaded, but FLUF UI's ui mode was not set to 'ImGui'");
    }

    SetupHooks();
    ReadUniverseIni();
    ReadFreelancerIni();
}

Retold::~Retold()
{
    if (flufUi && flufUi->GetConfig()->uiMode == UiMode::ImGui)
    {
        flufUi->GetImGuiInterface()->UnregisterImGuiModule(this);
    }
}

std::string_view Retold::GetModuleName() { return moduleName; }

SETUP_MODULE(Retold);
