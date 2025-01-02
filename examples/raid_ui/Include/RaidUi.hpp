#pragma once

#include "FlufModule.hpp"
#include "ImGui/ImGuiModule.hpp"
#include "RmlUi/Core/DataModelHandle.h"
#include "RmlUi/Core/ElementDocument.h"
#include "vendor/RmlUi/Source/Core/Memory.h"

#include <memory>

class FlufUi;

struct GroupMember
{
        std::string name;
        uint shipClass;
        uint shipArch;
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

class RaidUi final : public FlufModule, public ImGuiModule
{
        std::shared_ptr<FlufUi> flufUi;

        Rml::ElementDocument* document = nullptr;
        Rml::UnorderedMap<uint, GroupMember> members;
        Rml::UnorderedMap<uint, std::string> shipImageMap;
        ShipClassImageMap shipClassImageMap;
        double timer = 5.0;
        bool imguiPanelLocked = true;

        static void RadialProgressBar(const std::string& label, float progress, const ImVec2& size, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      ImVec2 center = ImVec2(0, 0));

        void OnFixedUpdate(double delta) override;
        void OnGameLoad() override;
        void Render() override;

    public:
        static constexpr std::string_view moduleName = "Raid UI";

        RaidUi();
        ~RaidUi() override;
        std::string_view GetModuleName() override;
};
