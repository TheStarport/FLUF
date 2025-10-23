#pragma once

#include "FlufModule.hpp"

#include <KeyManager.hpp>
#include <memory>

class FlufUi;

struct CustomStrafe
{
        float acceleration;
        uint leftFuse;
        uint rightFuse;
        float verticalMultiplier;
        float thrusterPenaltyMultiplier;
};

class SmoothStrafing final : public FlufModule
{
        inline static std::unordered_map<uint, CustomStrafe> shipStrafeForces;
        inline static std::unordered_map<uint, float> thrusterPenaltyReductions;
        inline static float totalTimeBeenStrafing = 0.f;
        inline static float deltaTime = 0.f;
        inline static float currentHorizontalStrafeForce = 0.f;
        inline static float currentVerticalStrafeForce = 0.f;
        inline static uint lastFuse = 0;
        inline static bool fuseIsLit = false;

        static float* __fastcall GetStrafeForce(CShip* ship, CStrafeEngine* engine);
        static void OnStrafeForceApply();
        static float* __fastcall GetThrusterForce(const Archetype::Thruster* archetype, const CEqObj* owner);
        static void OnThrusterForceApply();

        void BeforePhysicsUpdate(uint system, float delta) override;
        static void ReadShipIniFile(INI_Reader& ini);
        static void ReadEquipmentIniFile(INI_Reader& ini);
        void OnGameLoad() override;
        void OnLaunch(uint client, struct FLPACKET_LAUNCH& launch) override;

    public:
        static constexpr std::string_view moduleName = "Smooth Strafing";

        SmoothStrafing();
        ~SmoothStrafing() override = default;
        std::string_view GetModuleName() override;
};
