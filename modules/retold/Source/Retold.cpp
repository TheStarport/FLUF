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

static int handler(std::string& iniBuffer, const char* section, const char* name, const char* value)
{
    if (!name && !value)
    {
        iniBuffer += std::format("[{}]\n", section);
        return 1;
    }

    iniBuffer += std::format("{} = {}\n", name, value);

    return 1;
}

const char* systemIniBuffer;
DWORD systemReturnAddress;
DWORD Retold::OnSystemIniOpen(INI_Reader& iniReader, const char* file, bool unk)
{
    if (!iniReader.open(file, unk))
    {
        return 0;
    }

    iniReader.close();

    static std::string systemBuffer;
    systemBuffer.clear();

    for (auto& overrideFile : systemFileOverrides)
    {
        if (_strcmpi(file, overrideFile.first.c_str()) == 0)
        {
            systemBuffer += overrideFile.second;
            break;
        }
    }

    Fluf::Info(file);
    ini_parse(file, reinterpret_cast<ini_handler>(handler), &systemBuffer);

    systemIniBuffer = systemBuffer.c_str();
    return systemBuffer.size();
}

void __declspec(naked) Retold::SystemIniOpenNaked()
{
    constexpr static DWORD openMemory = 0x0630FC50;
    constexpr static DWORD open = 0x630F9B0;
    __asm
    {
        mov eax, [esp]
        mov systemReturnAddress, eax
        push ecx // Store for restoration
        push [esp+12] // bool unk
        push [esp+12] // const char* path
        push ecx // INI_Reader
        mov ecx, Retold::instance
        call OnSystemIniOpen // should pop 12
        test eax, eax
        jz normal_operation

        pop ecx
        add esp, 12 // Remove the previous two parameters + return address
        push eax
        push systemIniBuffer
        push systemReturnAddress
        jmp openMemory

        normal_operation:
            pop ecx
            jmp open
    }
}

void Retold::HookSystemFileReading()
{
    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    static auto systemIniOpenRedirectionAddress = &SystemIniOpenNaked;
    static auto systemIniOpenRedirectionAddress2 = &systemIniOpenRedirectionAddress;
    MemUtils::WriteProcMem(fl + 0x15379D, &systemIniOpenRedirectionAddress2, sizeof(systemIniOpenRedirectionAddress2));
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

bool Retold::BeforeBaseExit(uint baseId, uint client)
{
    auto system = const_cast<Universe::ISystem*>(Universe::get_system(CreateID("li01")));
    auto base = const_cast<Universe::IBase*>(Universe::get_base(CreateID("li01_01_base")));

    return true;
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
        contentStoryCreateDetour = nullptr;
    }
}

class CliEquip
{
    public:
        virtual void dunno0();
        virtual void dunno4();
        virtual void dunno8();
        virtual void fire(const Vector& pos);
        virtual void fireForward();

        CEquip* equip;
        Ship* owner;
};

void Retold::OnFixedUpdate(const double delta)
{
    static int counter = 0;

    if (counter++ > 60)
    {
        counter = 0;
    }

    auto iobj = Fluf::GetPlayerIObj();
    if (!iobj)
    {
        return;
    }
    st6::list<CliEquip*>* equipList = (st6::list<CliEquip*>*)(DWORD(iobj) + (4 * 45));

    for (auto equip : *equipList)
    {
        if (equip->equip->archetype->get_class_type() == Archetype::ClassType::Gun
            && equip->equip->IsActive())
        {
            equip->fire({ 0, 0, 0 });
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

    HookSystemFileReading();

    INI_Reader ini;
    if (!ini.open("../DATA/UNIVERSE/universe.ini", false))
    {
        throw ModuleLoadException("universe.ini not found");
    }

    while (ini.read_header())
    {
        if (!ini.is_header("system"))
        {
            continue;
        }

        std::string systemFile{};
        std::list<std::string> fileOverrides;
        while (ini.read_value())
        {
            if (ini.is_value("file"))
            {
                systemFile = ini.get_value_string();
            }
            else if (ini.is_value("file_override"))
            {
                fileOverrides.push_back(ini.get_value_string());
            }
        }

        if (!systemFile.empty() && !fileOverrides.empty())
        {
            std::string overrideFile;

            for (auto& filePath : fileOverrides)
            {
                if (!std::filesystem::exists(filePath))
                {
                    filePath = std::string("..\\DATA\\UNIVERSE\\") + filePath;
                }

                Fluf::Debug(std::format("Loading override file: {}", filePath));

                std::ifstream file(filePath, std::ios::in | std::ios::binary);
                if (!file)
                {
                    continue;
                }

                const auto sz = std::filesystem::file_size(filePath);
                std::string result(sz, '\0');
                file.read(result.data(), sz);
                file.close();

                overrideFile += result + "\n";
            }

            if (!overrideFile.empty())
            {
                systemFileOverrides[std::format("..\\data\\universe\\{}", systemFile)] = overrideFile;
            }
        }
    }
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
