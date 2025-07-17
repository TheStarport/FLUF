#pragma once

#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "ImGui/ImGuiModule.hpp"
#include "Utils/MemUtils.hpp"
#include "PatcherConfig.hpp"

class FlufUi;
class QolPatcher final : public FlufModule
{
        class MemoryPatch
        {
                std::string moduleName;
                DWORD patchOffset;
                std::vector<byte> oldData;
                std::vector<byte> patchedData;
                bool patched = false;
                bool requiresRestart = false;

            public:
                MemoryPatch(const std::string& module, DWORD offset, const std::initializer_list<byte> newData);
                void Patch();
                void Unpatch();
                static std::shared_ptr<MemoryPatch> Create(const std::string& module, DWORD offset, const std::initializer_list<byte> newData);
        };

        struct Option
        {
                std::string name;
                std::string description;
                std::vector<std::shared_ptr<MemoryPatch>> patches;
                bool requiresRestart = false;
                bool* flag = nullptr;

                void Patch();
                void Unpatch();
                Option(const std::string& name, const std::string& description, bool* configFlag, bool requiresRestart,
                       std::initializer_list<std::shared_ptr<MemoryPatch>> patches);
        };

        std::shared_ptr<FlufUi> flufUi;
        rfl::Box<PatcherConfig> config;
        std::unordered_map<std::string, std::vector<Option>> memoryPatches; // Patches split by category

        void Render(bool saveRequested);
        void TogglePatches(bool state);
        void OnLogin(uint client, bool singlePlayer, FLPACKET_UNKNOWN*) override;
        void OnGameLoad() override;

    public:
        static constexpr std::string_view moduleName = "qol_patcher";

        QolPatcher();
        ~QolPatcher() override;
        std::string_view GetModuleName() override;
};
