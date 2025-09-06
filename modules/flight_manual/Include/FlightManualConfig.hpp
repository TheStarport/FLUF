#pragma once
#include <vector>
#include <rfl/Skip.hpp>

struct FlightManualPage
{
        std::string title;
        std::string icon;
        std::string path;
        std::string content;
        rfl::Skip<std::vector<FlightManualPage*>> children;
};

struct FlightManualConfig
{
        static constexpr char path[] = "modules/config/flight_manual.yml";
        std::string rootPageContent = "This text will appear on the root page";
        std::vector<FlightManualPage> pages{ {} };
};
