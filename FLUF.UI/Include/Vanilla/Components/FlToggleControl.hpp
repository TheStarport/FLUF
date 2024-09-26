#pragma once
#include "Structures.hpp"

class FlToggleControl final : public FlControl
{
        byte data[972]{}; // 0x3CC in sub_55D3C0
        FlToggleControl() = default;

    public:
        bool GetToggleState() { return *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x3A0); }
        void SetToggleState(const bool value) { *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x3A0) = value; }
};
