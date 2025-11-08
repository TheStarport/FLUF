#pragma once

#include "FlufModule.hpp"

#include <KeyManager.hpp>
#include <memory>
#include <ImGui/ImGuiModule.hpp>

class ImGuiInterface;
class FlufUi;

struct Objective
{
        std::string icon;
        std::string message;
        bool complete = false;
};

class ObjectiveTracking final : public FlufModule, protected ImGuiModule
{
        ImGuiInterface* imgui{};
        std::vector<Objective> objectives;

        void Render() override;
        void OnGameLoad() override;

    public:
        static constexpr std::string_view moduleName = "Objective Tracking";

        ObjectiveTracking();
        ~ObjectiveTracking() override;
        std::string_view GetModuleName() override;
};
