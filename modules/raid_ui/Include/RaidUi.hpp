#pragma once

#include "FlufModule.hpp"
#include "ImGui/ImGuiModule.hpp"

#include <KeyManager.hpp>
#include <memory>

class FlufUi;

struct GroupMember
{
        std::string name;
        uint shipClass;
        float distance;
        float healthPercent;
        float healthCurrent;
        float healthMax;
        float shieldPercent;
        float shieldCurrent;
        float shieldMax;
        bool shieldRecharging;
        float shieldRechargeStart;
        float shieldRechargeEnd;
        double deathTimer = 0.f;
};

struct ShipClassImageMap
{
        std::string folderName = R"(..\DATA\INTERFACE\IMAGES\SYMBOLS)";
        std::unordered_map<uint, std::string> shipClassImageMap{
            { 0,        "lf.png" },
            { 1,        "hf.png" },
            { 2, "freighter.png" },
            { 3,       "vhf.png" },
        };
};

struct CustomisationSettings
{
        bool enable = true;
        bool hideLockSymbol = false;
        bool debugMode = false;
        float refreshRate = 0.25f;
        std::array<float, 4> nameColor = { 0.8f, 0.35f, 1.f, 1.f };
        std::array<float, 4> progressBarTextColor = { 0.654f, 0.653f, 0.356f, 1.f };
        std::array<float, 4> healthBarColor = { 0.47f, 0.16f, 0.2f, 1.f };
        std::array<float, 4> shieldBarColor = { 1.f, 0.47f, 0.05f, 1.f };
        std::array<float, 4> shieldBarRechargingColor = { 0.243f, 0.24f, 0.7f, 1.f };
};

class RaidUi final : public FlufModule, public ImGuiModule
{
        std::shared_ptr<FlufUi> flufUi;

        std::unordered_map<uint, GroupMember> members;
        ShipClassImageMap shipClassImageMap;
        double timer = 5.0;
        bool imguiPanelLocked = true;

        static constexpr char customisationFile[] = "raid_ui_options.yml";
        rfl::Box<CustomisationSettings> customisationSettings;

        static void RadialProgressBar(const std::string& label, float progress, const ImVec2& size, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      ImVec2 center = ImVec2(0, 0));

        void OnFixedUpdate(double delta) override;
        void OnGameLoad() override;
        void Render() override;

        bool OnTogglePanelKeyCommand(KeyState state);
        void RenderRaidUiOptions(bool saveRequested);

    public:
        static constexpr std::string_view moduleName = "Raid UI";

        RaidUi();
        ~RaidUi() override;
        std::string_view GetModuleName() override;
};
