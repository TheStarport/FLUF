#pragma once

#include "FlufModule.hpp"

#include <KeyManager.hpp>
#include <memory>
#include <ImGui/ImGuiModule.hpp>
#include <Internal/ImAnim/Easing.hpp>
#include <Internal/ImAnim/FloatAnim.hpp>
#include <Internal/ImAnim/ImVec2Anim.hpp>

class ImGuiInterface;
class FlufUi;

class ObjectiveTracking;
class Objective
{
        friend ObjectiveTracking;

        bool crossOut = false;
        bool complete = false;
        float opacity = 1.0f;
        std::shared_ptr<imanim::FloatAnim> animation;

    public:
        std::string icon;
        std::string message;

        Objective(const std::string& icon, const std::string& message) : icon(icon), message(message) {}

        void MarkComplete(const bool crossOut = false)
        {
            complete = true;
            animation->Start();

            this->crossOut = crossOut;
        }
};

class ObjectiveTracking final : public FlufModule, protected ImGuiModule
{
        ImGuiInterface* imgui{};
        std::vector<std::shared_ptr<Objective>> objectives;

        void Render() override;
        void OnFixedUpdate(const float delta, bool gamePaused) override;
        void OnGameLoad() override;

    public:
        static constexpr std::string_view moduleName = "Objective Tracking";

        ObjectiveTracking();
        ~ObjectiveTracking() override;
        std::string_view GetModuleName() override;

        std::weak_ptr<Objective> AddObjective(std::string_view icon, std::string_view message);
};
