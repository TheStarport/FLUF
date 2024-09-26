#pragma once
#include "Structures.hpp"

class FlScrollControl final : public FlControl
{
        byte data[1072] = {}; // 0x430 in sub_597AB0
        FlScrollControl() = default;

    public:
        float GetCurrentPosition() { return *reinterpret_cast<float*>(reinterpret_cast<unsigned int>(this) + 0x3C8); }
        void SetCurrentPosition(const unsigned int value) { Perform(0x38, value, 0); }
        void SetCurrentCapacity(const unsigned int cap) { Perform(0x37, 1, cap); }
};
