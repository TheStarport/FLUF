#pragma once

#include "Structures.hpp"

class FlButtonControl final : public FlControl
{
        byte data[888]{}; // 0x378 in sub_55D2B0
        FlButtonControl() = default;
};
