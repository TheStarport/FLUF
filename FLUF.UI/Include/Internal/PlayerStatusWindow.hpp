#pragma once

#include "ImGui/ImGuiInterface.hpp"

class CustomHud;
class PlayerStatusWindow
{
        friend CustomHud;

        bool windowOpen = false;
        std::unordered_map<FlufModule*, RegisterMenuFunc> registeredMenus;

    public:
        PlayerStatusWindow();
        void RegisterNewMenu(FlufModule* module, RegisterMenuFunc func);
        void Render(const std::unordered_map<std::string, std::unordered_map<FlufModule*, OnRenderStatsMenu>>& statsMenus);
};
