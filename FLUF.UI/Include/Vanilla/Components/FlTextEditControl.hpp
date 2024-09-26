#pragma once
#include "FlTextControl.hpp"

class FlTextEditControl final : public FlTextControl
{
        byte data[1000] = {};
        FlTextEditControl() = default;

        using SetupTextEditControlFunction = void(__thiscall*)(FlTextEditControl* control);

    public:
        constexpr int MaxCharacterCount = 32;
        constexpr float DefaultTextEditValue1 = 0.52f;
        constexpr float DefaultTextEditValue2 = 0.03f;
        const auto SetupTextEditControl = reinterpret_cast<SetupTextEditControlFunction>(0x57C750);
};
