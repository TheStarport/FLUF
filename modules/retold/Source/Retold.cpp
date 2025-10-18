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

FireResult __thiscall Retold::GunCanFireDetour(CEGun* gun, Vector& target)
{
    RetoldHooks::gunCanFireDetour.UnDetour();
    auto canFire = RetoldHooks::gunCanFireDetour.GetOriginalFunc()(gun, target);
    RetoldHooks::gunCanFireDetour.Detour(GunCanFireDetour);

    auto gunInfo = instance->extraWeaponData.find(gun->archetype->archId);
    if (canFire != FireResult::Success || gunInfo == instance->extraWeaponData.end())
    {
        return canFire;
    }

    auto& em = gun->owner->equipManager;
    const CEShield* shield = static_cast<CEShield*>(em.FindFirst(static_cast<uint>(EquipmentClass::Shield)));

    if (!shield || shield->currShieldHitPoints < gunInfo->second.shieldPowerUsage)
    {
        return FireResult::PowerRequirementsNotMet;
    }

    return FireResult::Success;
}

void __thiscall Retold::LauncherConsumeFireResourcesDetour(CELauncher* launcher)
{
    RetoldHooks::consumeFireResourcesDetour.UnDetour();
    RetoldHooks::consumeFireResourcesDetour.GetOriginalFunc()(launcher);
    RetoldHooks::consumeFireResourcesDetour.Detour(LauncherConsumeFireResourcesDetour);

    auto gunInfo = instance->extraWeaponData.find(launcher->archetype->archId);
    if (gunInfo == instance->extraWeaponData.end())
    {
        return;
    }

    auto& em = launcher->owner->equipManager;
    if (CEShield* shield = static_cast<CEShield*>(em.FindFirst(static_cast<uint>(EquipmentClass::Shield))))
    {
        shield->currShieldHitPoints = std::clamp(shield->currShieldHitPoints - gunInfo->second.shieldPowerUsage, 0.f, shield->maxShieldHitPoints);
    }
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

void Retold::OnServerStart(const SStartupInfo& startup_info) { HookContentDll(); }

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

void Retold::OnFixedUpdate(const double delta)
{
    if (!autoTurretsEnabled)
    {
        return;
    }

    auto iObj = Fluf::GetPlayerIObj();
    if (!iObj)
    {
        return;
    }

    auto* equipList = reinterpret_cast<st6::list<CliLauncher*>*>(reinterpret_cast<DWORD>(iObj) + (4 * 45));

    IObjRW* target = nullptr;
    iObj->get_target(target);
    if (!target)
    {
        return;
    }

    float attitude = 0.0f;
    target->get_attitude_towards(attitude, reinterpret_cast<const IObjInspect*>(iObj));

    if (attitude > -0.6f)
    {
        return;
    }

    for (const auto equip : *equipList)
    {
        if (equip->launcher->archetype->get_class_type() == Archetype::ClassType::Gun && equip->launcher->IsActive())
        {
            const auto gun = dynamic_cast<CEGun*>(equip->launcher);
            if (!gun->GunArch()->autoTurret)
            {
                continue;
            }

            if (Vector targetPos{}; iObj->cship()->get_tgt_lead_fire_pos(targetPos) && equip->fire(targetPos))
            {
                auto pos = gun->GetBarrelPosWS(0);
                const auto mult = 1.0f / static_cast<float>(gun->GetProjectilesPerFire());
                pos *= mult;
                CliLauncher::PlayFireSound(equip, pos, nullptr);
            }
        }
    }
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
