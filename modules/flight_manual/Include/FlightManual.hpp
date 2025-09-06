#pragma once

#include "FlufModule.hpp"
#include "ImGui/ImGuiModule.hpp"
#include "Vanilla/AbstractVanillaInterface.hpp"

#include <FlightManualConfig.hpp>
#include <rfl/Ref.hpp>

class FlightManualWindow;
class FlufUi;

class FlightManual final : public FlufModule, public ImGuiModule, public AbstractVanillaInterface
{
        std::shared_ptr<FlufUi> flufUi;
        std::unique_ptr<FlightManualWindow> flightManualWindow;
        rfl::Ref<FlightManualConfig> config;

        void OnGameLoad() override;
        void Render() override;

    public:
        static constexpr std::string_view moduleName = "Flight Manual";

        FlightManual();
        ~FlightManual() override;
        std::string_view GetModuleName() override;
};
