#pragma once

#include "Structures.hpp"

class FlButtonControl final : public FlControl
{
        FlButtonControl() = default;

    public:
        byte data[0xCC]{};
        char iconFile[100]{};
        byte data2[0x248]{};
};
