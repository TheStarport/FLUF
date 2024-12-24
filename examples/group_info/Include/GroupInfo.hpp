#pragma once

#include "FlufModule.hpp"
#include "RmlUi/Core/DataModelHandle.h"
#include "RmlUi/Core/ElementDocument.h"
#include "vendor/RmlUi/Source/Core/Memory.h"

#include <memory>

class FlufUi;

struct GroupMember
{
        Rml::String name;
        uint shipArch;
        float distance;
        float health;
        float shield;
        double deathTimer = 0.f;
};

class GroupInfo final : public FlufModule
{
        std::shared_ptr<FlufUi> flufUi;

        Rml::ElementDocument* document = nullptr;
        Rml::DataModelHandle memberDataModel;
        Rml::UnorderedMap<uint, GroupMember> members;
        double timer = 5.0;

        void OnFixedUpdate(const double delta) override;
        void OnGameLoad() override;

    public:
        static constexpr std::string_view moduleName = "group_info";

        GroupInfo();
        std::string_view GetModuleName() override;
};
