#pragma once
#include "Structures.hpp"

class FlWireControl final : public FlControl
{
        byte data[912] = {}; // 0x390 in sub_5A4E10
        FlWireControl() = default;

        using WireControlFunc = FlWireControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* sound, const Vector& position,
                                                            const Vector& dimensions, const char* hardpoint, uint u2, uint resHintId, uint u4, uint u5);

    public:
        inline static auto CreateWireControl = reinterpret_cast<WireControlFunc>(0x5A0D00);
};
