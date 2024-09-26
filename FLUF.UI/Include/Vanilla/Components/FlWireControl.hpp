#pragma once
#include "Structures.hpp"

class FlWireControl final : public FlControl
{
        byte data[912] = {}; // 0x390 in sub_5A4E10
        FlWireControl() = default;
};
