#pragma once

#include "FlufModule.hpp"

#include <KeyManager.hpp>
#include <memory>

class FlufUi;

class SmoothStrafing final : public FlufModule
{
        inline static std::unordered_map<uint, float> shipStrafeForces;
        inline static float totalTimeBeenStrafing = 0.f;
        inline static float deltaTime = 0.f;
        inline static float currentStrafeForce = 0.f;

        static float* __fastcall GetStrafeForce(const CShip* ship);
        static void OnStrafeForceApply();
        static float* __fastcall GetThrusterForce(const Archetype::Thruster* archetype, const CEqObj* owner);
        static void OnThrusterForceApply();

        void BeforePhysicsUpdate(uint system, float delta) override;
        static void ReadIniFile(INI_Reader& ini);
        void OnGameLoad() override;
        void OnLaunch(uint client, struct FLPACKET_LAUNCH& launch) override;

    public:
        static constexpr std::string_view moduleName = "Smooth Strafing";

        SmoothStrafing();
        ~SmoothStrafing() override = default;
        std::string_view GetModuleName() override;
};
