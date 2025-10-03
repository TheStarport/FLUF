#pragma once

#include "FlufModule.hpp"

#include <KeyManager.hpp>
#include <memory>

class FlufUi;

class SmoothStrafing final : public FlufModule
{
        std::unordered_map<uint, std::pair<float, float>> shipStrafeForces;
        StrafeDir currentStrafeDir = StrafeDir::None;
        float currentStrafeForce = 0.f;

        bool OnStrafeLeftKey(KeyState keyState);
        bool OnStrafeRightKey(KeyState keyState);

        void ReadIniFile(INI_Reader& ini);
        void OnGameLoad() override;
        void OnLaunch(uint client, struct FLPACKET_LAUNCH& launch) override;
        void OnPhysicsUpdate(uint system, float delta) override;

    public:
        static constexpr std::string_view moduleName = "Smooth Strafing";

        SmoothStrafing();
        ~SmoothStrafing() override = default;
        std::string_view GetModuleName() override;
};
