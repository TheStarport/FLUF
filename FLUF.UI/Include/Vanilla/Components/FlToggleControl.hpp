#pragma once
#include "Structures.hpp"

class FlToggleControl final : public FlControl
{
        byte data[972]{}; // 0x3CC in sub_55D3C0
        FlToggleControl() = default;

        using CreateToggleControlFunction = FlToggleControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* enabled,
                                                                          const char* disabled, const char* sound, uint u1, const Vector& position,
                                                                          const char* hardpoint, const Vector& mouseSize, uint u2, uint u3, uint u4, uint u5,
                                                                          uint enabledResStrId, uint disabledResStrId);

    public:
        bool GetToggleState() { return *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x3A0); }
        void SetToggleState(const bool value) { *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x3A0) = value; }

        inline static auto CreateToggleControl = reinterpret_cast<CreateToggleControlFunction>(0x5A0FB0);
};
