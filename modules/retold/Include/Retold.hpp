#pragma once

#include "FlufModule.hpp"

#include <KeyManager.hpp>
#include <memory>
#include <ImGui/ImGuiModule.hpp>

class EquipmentDealerWindow;
class FlufUi;

struct ContentStory
{
        void* vtable00;
        void* vtable04;
        DWORD dunno08;
        uint missionStage;
        DWORD dunno10;
        float dunno14;
        bool dunno18;
        const char name[32];
};

class Retold final : public FlufModule, public ImGuiModule
{
        std::shared_ptr<FlufUi> flufUi = nullptr;
        std::shared_ptr<EquipmentDealerWindow> equipmentDealerWindow = nullptr;
        DWORD contentDll = 0;
        inline static Retold* instance = nullptr;

        ContentStory* contentStory = nullptr;

        template <typename T>
        using CreateContentMessageHandler = T(__thiscall*)(T, void* contentInstance, DWORD* payload);

        std::unique_ptr<FunctionDetour<CreateContentMessageHandler<ContentStory*>>> contentStoryCreateDetour;

        static ContentStory* __thiscall ContentStoryCreateDetour(ContentStory* story, void* contentInstance, DWORD* payload);

        void HookContentDll();

        void OnGameLoad() override;
        void OnServerStart(const SStartupInfo&) override;
        void Render() override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnDllUnloaded(std::string_view dllName, HMODULE dllPtr) override;

    public:
        static constexpr std::string_view moduleName = "Retold";

        Retold();
        ~Retold() override;
        std::string_view GetModuleName() override;
};
