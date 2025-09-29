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
                DWORD moduleAddress;
                std::string moduleName;
                std::vector<DWORD> patchOffsets;
                std::vector<byte> oldData;
                std::vector<byte> patchedData;
                bool patched = false;
                bool requiresRestart = false;

            public:
                virtual ~MemoryPatch() = default;
                MemoryPatch(const std::string& module, std::initializer_list<DWORD> offsets, size_t patchSize, const std::initializer_list<byte> newData);
                virtual void Patch();
                void Unpatch();
                virtual void RenderComponent() {};
                [[nodiscard]]
                std::string GetPatchId() const;
                size_t GetPatchSize() const;
                byte* GetPatchData();
        };

        class ColorPatch final : public MemoryPatch
        {
                DWORD* newColor;
                bool bgr = true;
                bool alpha = false;

            public:
                ColorPatch(const std::string& moduleName, std::initializer_list<DWORD> offsets, DWORD* newColor, bool isBgr = true, bool includeAlpha = false);
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
                OptionPatch(const std::string& moduleName, std::initializer_list<DWORD> offsets, size_t patchSize, int* optionIndex,
                            std::initializer_list<PatchOption> patches);
                void RenderComponent() override;
        };

        class ReusablePatch final : public MemoryPatch
        {
                MemoryPatch* patch;

            public:
                ReusablePatch(const std::string& moduleName, std::initializer_list<DWORD> offsets, MemoryPatch* patch)
                    : MemoryPatch(moduleName, offsets, patch->GetPatchSize(), {}), patch(patch)
                {}

                void Patch() override
                {
                    memcpy(patchedData.data(), patch->GetPatchData(), patch->GetPatchSize());
                    MemoryPatch::Patch();
                }

                void RenderComponent() override { memcpy(patchedData.data(), patch->GetPatchData(), patch->GetPatchSize()); }
        };

        template <typename T>
        class ValuePatch final : public MemoryPatch
        {
                T* newValue;
                T min;
                T max;
                T defaultValue;

            public:
                ValuePatch(const std::string& moduleName, std::initializer_list<DWORD> offsets, T* newValue, T min, T max)
                    : MemoryPatch(moduleName, offsets, sizeof(T), {}), newValue(newValue), min(min), max(max)
                {
                    if (oldData.empty())
                    {
                        return;
                    }

                    defaultValue = *reinterpret_cast<T*>(oldData.data());

                    memcpy(patchedData.data(), newValue, sizeof(T));
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

        union RawValue {
                float f;
                int i;
                uint u;
                byte b[4];

                explicit RawValue(float f) : f(f) {}
                explicit RawValue(int i) : i(i) {}
                explicit RawValue(uint u) : u(u) {}

                std::initializer_list<byte> data() const { return std::initializer_list(b, b + sizeof(b)); }
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
        bool BeforeLaunchComplete(uint baseId, uint shipId) override;

        void RegisterHudPatches();
        void RegisterDisplayPatches();
        void RegisterChatPatches();
        void RegisterControlPatches();

    public:
        static constexpr std::string_view moduleName = "QoL Patcher";

        QolPatcher();
        ~QolPatcher() override;
        std::string_view GetModuleName() override;
};

#define PATCH(module, offset, ...)                                                                                     \
    new MemoryPatch                                                                                                    \
    {                                                                                                                  \
        module, offset, std::initializer_list<byte>{ __VA_ARGS__ }.size(), std::initializer_list<byte> { __VA_ARGS__ } \
    }
#define OPTION(name, description, flag, restart, ...) \
    category.emplace_back(name, description, flag, restart, std::initializer_list<MemoryPatch*>{ __VA_ARGS__ })
