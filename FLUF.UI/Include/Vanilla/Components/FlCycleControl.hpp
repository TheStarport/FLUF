#pragma once
#include "Structures.hpp"

class FlCycleControl final : public FlControl
{
        byte data[888] = {}; // 0x378 in sub_55D2B0
        FlCycleControl() = default;
        using CreateFlCycleControlFunction = FlCycleControl*(__thiscall*)(const FlControl* parent, const char* nickname, const Vector& position,
                                                                          float distanceAwayFromEachOther, float scale, float u1, float u2, float u3,
                                                                          bool rotateHorizontal);

    public:
        byte UpOrLeft() { return *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x34C); }

        // If flipped horizontally, this will be right
        byte DownOrRight() { return *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x360); }

        inline static auto CreateCycleControl = reinterpret_cast<CreateFlCycleControlFunction>(0x5A1870);
};
