#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "Retold.hpp"

#include <FLUF.UI.hpp>

#include <ImGui/ImGuiInterface.hpp>
#include <EquipmentDealerWindow.hpp>
#include <Utils/MemUtils.hpp>

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

void test(DWORD* ecx, const char* file, DWORD unk) { printf("%p, %s | %x\n", ecx, file, unk); }

byte* systemIniBuffer;
DWORD Retold::OnSystemIniOpen(INI_Reader* ini, const char* file, bool unk)
{
    printf("%p, %s | %s\n", ini, file, unk ? "Open" : "Close");
    return 0;
}

void __declspec(naked) Retold::SystemIniOpenNaked()
{
    static auto openMemory = &INI_Reader::open_memory;
    static auto open = &INI_Reader::open;
    __asm
    {
        push ecx // Store for restoration
        push [esp+12] // bool unk
        push [esp+12] // const char* path
        push ecx // INI_Reader
        mov ecx, Retold::instance
        call OnSystemIniOpen // should pop 12
        test eax, eax
        jz normal_operation

        pop ecx
        add esp, 8 // Remove the previous two parameters
        push eax
        push systemIniBuffer
        jmp openMemory

        normal_operation:
            pop ecx
            jmp open
    }
}

void Retold::HookSystemFileReading()
{
    const auto common = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll"));
    MemUtils::PatchCallAddr(common, 0xD72CE, SystemIniOpenNaked);
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
        contentStoryCreateDetour = nullptr;
    }
}

Retold::Retold()
{
    instance = this;
    HookSystemFileReading();

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
