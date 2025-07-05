#pragma once
#include <rfl/Timestamp.hpp>

struct PatchNote
{
        rfl::Timestamp<"%Y-%m-%d"> date;
        std::string content;
        std::string preamble;
        std::string version;
};
