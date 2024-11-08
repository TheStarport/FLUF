#pragma once
#include "Structures.hpp"

class FlStaticControl final : public FlControl
{
        byte data[1056]{}; // 0x420 in sub_589A20
        FlStaticControl() = default;
};
