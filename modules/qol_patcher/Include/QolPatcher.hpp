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
            protected:
                std::string moduleName;
                DWORD patchOffset;
                std::vector<byte> oldData;
                std::vector<byte> patchedData;
                bool patched = false;
                bool requiresRestart = false;

            public:
                virtual ~MemoryPatch() = default;
                MemoryPatch(const std::string& module, DWORD offset, const std::initializer_list<byte> newData);
                void Patch();
                void Unpatch();
                virtual void RenderComponent() {};
                [[nodiscard]]
                std::string GetPatchId() const;
        };

        class ColorPatch final : public MemoryPatch
        {
                DWORD* newColor;
                bool bgr = true;
                bool alpha = false;

            public:
                ColorPatch(const std::string& moduleName, DWORD offset, DWORD* newColor, bool isBgr = true, bool includeAlpha = false);
                void RenderComponent() override;
        };

        class OptionPatch final : public MemoryPatch
        {
            public:
                struct PatchOption
                {
                        std::string label;
                        std::vector<byte> data;
                };

            private:
                std::vector<PatchOption> patches{};
                int* selectedPatch;
                size_t patchSize;

            public:
                OptionPatch(const std::string& moduleName, DWORD offset, int* optionIndex, std::initializer_list<PatchOption> patches);
                void RenderComponent() override;
        };

        template <typename T>
        class ValuePatch final : public MemoryPatch
        {
                T* newValue;
                T min;
                T max;
                T defaultValue;

            public:
                ValuePatch(const std::string& moduleName, const DWORD offset, T* newValue, T min, T max)
                    : MemoryPatch(moduleName, offset, {}), newValue(newValue), min(min), max(max)
                {
                    const auto module = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName.empty() ? nullptr : moduleName.c_str()));
                    if (!module)
                    {
                        return;
                    }

                    const auto address = module + patchOffset;
                    oldData.resize(sizeof(T));
                    patchedData.resize(sizeof(T));
                    MemUtils::ReadProcMem(address, oldData.data(), oldData.size());

                    defaultValue = *reinterpret_cast<T*>(oldData.data());

                    // Default to the original color if none provided
                    if (!*newValue)
                    {
                        *newValue = defaultValue;
                        patchedData = oldData;
                    }
                    else
                    {
                        memcpy(patchedData.data(), newValue, sizeof(T));
                    }
                }

                void RenderComponent() override
                {
                    T value = *newValue;

                    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
                    {
                        float f = static_cast<float>(value);
                        ImGui::SliderFloat("##value", &f, static_cast<float>(min), static_cast<float>(max));
                        value = static_cast<T>(f);
                    }
                    else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int> || std::is_same_v<T, long> || std::is_same_v<T, unsigned long>)
                    {
                        int i = static_cast<int>(value);
                        ImGui::SliderInt("##value", &i, static_cast<int>(min), static_cast<int>(max));
                        value = static_cast<T>(i);
                    }

                    if (*newValue != defaultValue)
                    {
                        ImGui::SameLine();
                        if (ImGui::Button("Reset##"))
                        {
                            value = defaultValue;
                        }
                    }

                    if (value != *newValue)
                    {
                        *newValue = value;
                        memcpy(patchedData.data(), newValue, sizeof(T));
                    }
                }
        };

        struct Option
        {
                std::string name;
                std::string description;
                std::vector<MemoryPatch*> patches;
                bool requiresRestart = false;
                bool* flag = nullptr;

                void Patch() const;
                void Unpatch() const;
                Option(const std::string& name, const std::string& description, bool* configFlag, bool requiresRestart,
                       std::initializer_list<MemoryPatch*> patches);
        };

        std::shared_ptr<FlufUi> flufUi;
        rfl::Box<PatcherConfig> config;
        std::unordered_map<std::string, std::vector<Option>> memoryPatches; // Patches split by category

        void Render(bool saveRequested);
        void TogglePatches(bool state);
        void OnLogin(uint client, bool singlePlayer, FLPACKET_UNKNOWN*) override;
        void OnCharacterSelect(uint client, FLPACKET_UNKNOWN*) override;
        void OnGameLoad() override;

        void RegisterHudPatches();
        void RegisterDisplayPatches();
        void RegisterChatPatches();
        void RegisterControlPatches();

    public:
        static constexpr std::string_view moduleName = "qol_patcher";

        QolPatcher();
        ~QolPatcher() override;
        std::string_view GetModuleName() override;
};

#define PATCH(module, offset, ...)                                  \
    new MemoryPatch                                                 \
    {                                                               \
        module, offset, std::initializer_list<byte> { __VA_ARGS__ } \
    }
#define OPTION(name, description, flag, restart, ...) \
    category.emplace_back(name, description, flag, restart, std::initializer_list<MemoryPatch*>{ __VA_ARGS__ })
