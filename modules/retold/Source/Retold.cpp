#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "Retold.hpp"

#include <FLUF.UI.hpp>

#include <ImGui/ImGuiInterface.hpp>
#include <EquipmentDealerWindow.hpp>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

ContentStory* __thiscall Retold::ContentStoryCreateDetour(ContentStory* story, void* contentInstance, DWORD* payload)
{
    instance->contentStory = story;
    instance->contentStoryCreateDetour->UnDetour();
    instance->contentStoryCreateDetour->GetOriginalFunc()(story, contentInstance, payload);
    instance->contentStoryCreateDetour->Detour(ContentStoryCreateDetour);
    return story;
}

void Retold::HookContentDll()
{
    contentDll = reinterpret_cast<DWORD>(GetModuleHandleA("content.dll"));
    if (!contentDll || contentStoryCreateDetour)
    {
        return;
    }

    contentStoryCreateDetour = std::make_unique<FunctionDetour<CreateContentMessageHandler<ContentStory*>>>(
        reinterpret_cast<CreateContentMessageHandler<ContentStory*>>(contentDll + 0x5E10));
    contentStoryCreateDetour->Detour(ContentStoryCreateDetour);
}

void Retold::OnGameLoad()
{
    HookContentDll();

    if (flufUi->GetConfig()->uiMode != UiMode::ImGui)
    {
        return;
    }

    auto imgui = flufUi->GetImGuiInterface();
    imgui->RegisterImGuiModule(this);
    //imgui->RegisterOptionsMenu(this, static_cast<RegisterOptionsFunc>(&Retold::RenderRetoldOptions));

    equipmentDealerWindow = std::make_unique<EquipmentDealerWindow>(imgui);
}

void Retold::OnServerStart(const SStartupInfo& startup_info) { HookContentDll(); }

int i = 0;
void Retold::Render()
{
    if (contentStory && i++ > 150)
    {
        i = 0;
        Fluf::Info(std::format("Story+08: {}", contentStory->dunno08));
        Fluf::Info(std::format("Story+10: {}", contentStory->dunno10));
        Fluf::Info(std::format("Story+14: {}", contentStory->dunno14));
        Fluf::Info(std::format("Story+18: {}", contentStory->dunno18));
        Fluf::Info(std::format("Story+24: {}", contentStory->name));
    }
    equipmentDealerWindow->Render();
}

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

    // TODO: Load config?
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
