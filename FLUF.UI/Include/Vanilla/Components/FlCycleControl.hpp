#pragma once
#include "Structures.hpp"

class FlCycleControl final : public FlControl
{
        byte data[888] = {}; // 0x378 in sub_55D2B0
        FlCycleControl() = default;

    public:
        byte UpOrLeft() { return *reinterpret_cast<byte*>(reinterpret_cast<uint>(this) + 0x34C); }

        // If flipped horizontally, this will be right
        byte DownOrRight() { return *reinterpret_cast<byte*>(reinterpret_cast<uint>(this) + 0x360); }
};
