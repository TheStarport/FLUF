#pragma once

#include "FlufModule.hpp"

#include <memory>
#include <ImGui/ImGuiModule.hpp>

class Debugger final : public FlufModule, ImGuiModule
{
        inline static Debugger* instance = nullptr;
        DWORD serverDll = 0;
        bool renderDebugCommandWindow = true;
        bool renderDebugSpaceWindow = true;

        void RenderDebugSpaceWindow(Ship* player);
        void RenderDebugCommandWindow();

        void OnGameLoad() override;
        void Render() override;

    public:
        static constexpr std::string_view moduleName = "Debugger";

        Debugger();
        ~Debugger() override;
        std::string_view GetModuleName() override;
};
