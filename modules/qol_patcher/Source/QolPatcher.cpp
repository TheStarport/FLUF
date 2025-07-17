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

QolPatcher::MemoryPatch::MemoryPatch(const std::string& module, DWORD offset, const std::initializer_list<byte> newData)
{
    moduleName = module;
    patchOffset = offset;
    this->requiresRestart = requiresRestart;
    patchedData = newData;
    oldData.resize(patchedData.size());
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

    const auto address = module + patchOffset;
    MemUtils::ReadProcMem(address, oldData.data(), oldData.size());
    MemUtils::WriteProcMem(address, patchedData.data(), patchedData.size());
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

    const auto address = module + patchOffset;
    MemUtils::WriteProcMem(address, oldData.data(), oldData.size());
}

std::shared_ptr<QolPatcher::MemoryPatch> QolPatcher::MemoryPatch::Create(const std::string& module, DWORD offset, const std::initializer_list<byte> newData)
{
    return std::make_shared<MemoryPatch>(module, offset, newData);
}

void QolPatcher::Option::Patch()
{
    if (!*flag)
    {
        return;
    }

    for (auto patch : patches)
    {
        patch->Patch();
    }
}

void QolPatcher::Option::Unpatch()
{
    for (auto patch : patches)
    {
        patch->Unpatch();
    }
}

QolPatcher::Option::Option(const std::string& name, const std::string& description, bool* configFlag, bool requiresRestart,
                           std::initializer_list<std::shared_ptr<MemoryPatch>> patches)
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

QolPatcher::QolPatcher()
{
    config = rfl::Box<PatcherConfig>::make(*ConfigHelper<PatcherConfig, PatcherConfig::path>::Load(true));

#define PATCH(module, offset, ...)                    MemoryPatch::Create(module, offset, { __VA_ARGS__ })
#define OPTION(name, description, flag, restart, ...) category.emplace_back(name, description, flag, restart, std::initializer_list{ __VA_ARGS__ })

    auto& category = memoryPatches["Display"];

    OPTION("Borderless Window",
           "Removes the window borders when in windowed mode",
           &config->borderlessWindow,
           true,
           PATCH("", 0x02477A, 0x00, 0x00),
           PATCH("", 0x002490D, 0x00, 0x00));

    TogglePatches(true);
};
QolPatcher::~QolPatcher() = default;

std::string_view QolPatcher::GetModuleName() { return moduleName; }

SETUP_MODULE(QolPatcher);
