#include "PCH.hpp"

#include "QolPatcher.hpp"

#include "ClientServerCommunicator.hpp"
#include "FLUF.UI.hpp"
#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "ImGui/ImGuiHelper.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "Exceptions.hpp"

#include <rfl/msgpack.hpp>

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

QolPatcher::MemoryPatch::MemoryPatch(const std::string& module, std::initializer_list<DWORD> offsets, size_t patchSize,
                                     const std::initializer_list<byte> newData)
{
    moduleName = module;
    patchOffsets = offsets;
    this->requiresRestart = requiresRestart;
    patchedData = newData;
    moduleAddress = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName.empty() ? nullptr : moduleName.c_str()));

    if (patchedData.empty())
    {
        if (!moduleAddress)
        {
            return;
        }

        for (const auto offset : patchOffsets)
        {
            const auto address = moduleAddress + offset;
            oldData.resize(patchSize);
            patchedData.resize(patchSize);
            MemUtils::ReadProcMem(address, oldData.data(), oldData.size());
        }
    }
    else
    {
        oldData.resize(patchedData.size());
    }
}

void QolPatcher::MemoryPatch::Patch()
{
    if (patched)
    {
        return;
    }

    const auto module = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName.empty() ? nullptr : moduleName.c_str()));
    if (!module)
    {
        return;
    }

    // We make the assumption that all offsets passed in have the same original value
    for (const auto offset : patchOffsets)
    {
        const auto address = module + offset;
        MemUtils::ReadProcMem(address, oldData.data(), oldData.size());
        MemUtils::WriteProcMem(address, patchedData.data(), patchedData.size());
    }

    patched = true;
}

void QolPatcher::MemoryPatch::Unpatch()
{
    if (!patched)
    {
        return;
    }

    patched = false;
    const auto module = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName.empty() ? nullptr : moduleName.c_str()));
    if (!module)
    {
        return;
    }

    for (const auto offset : patchOffsets)
    {
        const auto address = module + offset;
        MemUtils::WriteProcMem(address, oldData.data(), oldData.size());
    }
}

std::string QolPatcher::MemoryPatch::GetPatchId() const { return std::to_string(patchOffsets.front()); }
size_t QolPatcher::MemoryPatch::GetPatchSize() const { return patchedData.size(); }
byte* QolPatcher::MemoryPatch::GetPatchData() { return patchedData.data(); }

void QolPatcher::Option::Patch() const
{
    if (!*flag)
    {
        return;
    }

    for (const auto& patch : patches)
    {
        patch->Patch();
    }
}

void QolPatcher::Option::Unpatch() const
{
    for (const auto& patch : patches)
    {
        patch->Unpatch();
    }
}

QolPatcher::Option::Option(const std::string& name, const std::string& description, bool* configFlag, bool requiresRestart,
                           std::initializer_list<MemoryPatch*> patches)
{
    this->name = name;
    this->description = description;
    flag = configFlag;
    this->requiresRestart = requiresRestart;
    this->patches = patches;
}

void QolPatcher::Render(bool saveRequested)
{
    ImGui::BeginTable("##qol-patcher-table", 2);

    for (auto& [category, patches] : memoryPatches)
    {
        ImGui::TableNextColumn();
        ImGui::SeparatorText(category.c_str());

        for (auto& option : patches)
        {
            ImGui::Checkbox(option.name.c_str(), option.flag);
            ImGui::TextWrapped(option.description.c_str());

            if (option.requiresRestart)
            {
                ImGui::SameLine();
                ImGuiHelper::HelpMarker("This change will not take effect until after a restart.", '*', 0xFF0000CC);
            }

            for (const auto& patch : option.patches)
            {
                ImGui::PushID(patch->GetPatchId().c_str());
                patch->RenderComponent();
                ImGui::PopID();
            }
        }
    }

    ImGui::EndTable();

    if (saveRequested)
    {
        ConfigHelper<PatcherConfig, PatcherConfig::path>::Save(*config, true);
        TogglePatches(false);
        TogglePatches(true);
    }
}

void QolPatcher::TogglePatches(bool state)
{
    for (auto& patches : memoryPatches | std::ranges::views::values)
    {
        for (auto& patch : patches)
        {
            if (state)
            {
                patch.Patch();
            }
            else
            {
                patch.Unpatch();
            }
        }
    }
}

void QolPatcher::OnLogin(uint client, bool singlePlayer, FLPACKET_UNKNOWN* flpacket_unknown)
{
    // We toggle them off then on to ensure any dll changes are properly tracked
    TogglePatches(false);
    TogglePatches(true);
}

void QolPatcher::OnCharacterSelect(uint client, FLPACKET_UNKNOWN* flpacket_unknown)
{
    // We do not have any hooks for detecting single player, so we hook on character select which also fires for loading saves
    TogglePatches(false);
    TogglePatches(true);
}

void QolPatcher::OnGameLoad()
{
    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        Fluf::Log(LogLevel::Error, "PatchNotes was loaded, but FLUF UI was not loaded. Crashes are likely.");
        return;
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    if (module->GetConfig()->uiMode != UiMode::ImGui)
    {
        throw ModuleLoadException("Patch Notes was loaded, but FLUF.UI's ui mode was not set to ImGui.");
    }

    const auto imgui = module->GetImGuiInterface();
    if (!imgui)
    {
        throw ModuleLoadException("Patch Notes was loaded, but FLUF.UI's failed to provide an ImGui interface.");
    }

    flufUi = module;

    imgui->RegisterOptionsMenu(this, reinterpret_cast<RegisterOptionsFunc>(&QolPatcher::Render));
}

bool QolPatcher::BeforeLaunchComplete(uint baseId, uint shipId)
{
    TogglePatches(false);
    TogglePatches(true);

    return true;
}

QolPatcher::QolPatcher()
{
    AssertRunningOnClient;

    config = rfl::Box<PatcherConfig>::make(*ConfigHelper<PatcherConfig, PatcherConfig::path>::Load(true));

    RegisterDisplayPatches();
    RegisterHudPatches();
    RegisterChatPatches();
    RegisterControlPatches();

    TogglePatches(true);
};

QolPatcher::~QolPatcher()
{
    for (const auto& options : memoryPatches | std::views::values)
    {
        for (auto& option : options)
        {
            option.Unpatch();

            for (const auto patch : option.patches)
            {
                delete patch;
            }
        }
    }
};

std::string_view QolPatcher::GetModuleName() { return moduleName; }

SETUP_MODULE(QolPatcher);
