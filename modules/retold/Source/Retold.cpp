#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "Retold.hpp"

#include <FLUF.UI.hpp>

#include <ImGui/ImGuiInterface.hpp>
#include <EquipmentDealerWindow.hpp>
#include <Utils/MemUtils.hpp>

#include <ini.h>

//static auto* shieldHitEffectLimit = reinterpret_cast<DWORD*>(0x613E60);
static auto* dacom = reinterpret_cast<DWORD*>(0x6789D0);

void* __stdcall Retold::GetShieldHitEffectArray(uint id)
{
    auto& entry = objectShieldHitEffectMap[id];
    return &entry.shieldHitArray[++entry.counter % MAX_COUNT_SHIELD_EFFECT];
}

void __declspec(naked) ShieldHitEffectsReplacement()
{
    static constexpr DWORD returnAddress = 0x53A0B8;
    static constexpr DWORD sub4F8110 = 0x4F8110;
    static constexpr DWORD sub4F7A90 = 0x4F7A90;
    static constexpr DWORD sub5B73E0 = 0x5B73E0;

    __asm
    {
        jle loc_53A061
        mov eax, [ebx+0x10]
        mov eax, [eax+0xB0]
        push eax
        call Retold::GetShieldHitEffectArray
        mov esi, eax
        mov edx, [esi]

        loc_53A061:
            mov ecx, [esi]
            test ecx, ecx
            jz      loc_53A094
            call    sub4F8110
            mov     ecx, [esi]
            mov     edx, [ecx]
            call    dword ptr [edx+4]
            mov     ecx, [esi]
            call    sub4F7A90
            mov     dword ptr [esi], 0

        loc_53A094:
            mov     ebp, [esp+0x9C-0x8C]
            mov     [esi], ebp
            mov     esi, [ebx+0x10]
            mov     edx, [dacom]
            mov     eax, [edx]
            add     esi, 8
            test    eax, eax
            jnz     back_to_fl
            call    sub5B73E0
            mov     [edx], eax

        back_to_fl:
            jmp returnAddress
    }
}

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
    const auto common = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll"));

    iniReaderOpenDetour.Detour(IniReaderOpenDetour);

    RetoldHooks::gunCanFireDetour.Detour(GunCanFireDetour);
    RetoldHooks::consumeFireResourcesDetour.Detour(LauncherConsumeFireResourcesDetour);
    RetoldHooks::shieldSetHealthDetour.Detour(ShieldSetHealthDetour);

    MemUtils::PatchAssembly(common + 0x3CF06, ShieldRegenerationPatchNaked);
    MemUtils::PatchAssembly(0x53A02A, ShieldHitEffectsReplacement);
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

void Retold::HookServerDll()
{
    serverDll = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));
    if (!serverDll)
    {
        return;
    }

    // patch Iobject->get_id() to return a nullptr instead of crashing, also speed it up a lot
    BYTE patch[]{ 0x8B, 0x80, 0xB0, 0x00, 0x00, 0x00, 0xC3 };
    MemUtils::WriteProcMem(DWORD(GetModuleHandleA("server")) + 0x61D3, patch, sizeof(patch));
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
    HookServerDll();

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
    HookServerDll();
}

void Retold::Render() { equipmentDealerWindow->Render(); }

void Retold::OnDllLoaded(std::string_view dllName, HMODULE dllPtr)
{
    if (dllName.ends_with("content.dll") || dllName.ends_with("Content.dll"))
    {
        HookContentDll();
    }

    if (dllName.ends_with("server.dll") || dllName.ends_with("Server.dll"))
    {
        HookServerDll();
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

    ProcessDotStacks(delta);
    ProcessHealingStacks(delta);
    RemoveShieldReductionStacks(delta);
    RemoveVulnerabilityStacks(delta);

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
