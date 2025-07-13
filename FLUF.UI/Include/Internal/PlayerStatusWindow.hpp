#pragma once

#include "ImGui/ImGuiInterface.hpp"
#include "ImGui/FlWindow.hpp"

class CustomHud;
class PlayerStatusWindow final : public FlWindow
{
        friend CustomHud;

        std::unordered_map<FlufModule*, RegisterMenuFunc> registeredMenus;
        const std::unordered_map<std::string, std::unordered_map<FlufModule*, OnRenderStatsMenu>>& statsMenus;

        void RenderWindowContents() override;

    public:
        void SetOpen();
        explicit PlayerStatusWindow(const std::unordered_map<std::string, std::unordered_map<FlufModule*, OnRenderStatsMenu>>&);
        void RegisterNewMenu(FlufModule* module, RegisterMenuFunc func);
};
