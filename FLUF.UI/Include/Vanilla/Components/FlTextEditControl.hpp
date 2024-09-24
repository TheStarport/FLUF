#pragma once
#include "FlTextControl.hpp"

class FlTextEditControl final : public FlTextControl
{
        byte data[1000] = {};
        FlTextEditControl() = default;

        using CreateTextEditControLFunction = FlTextEditControl*(__thiscall*)(const FlControl* parent, const char* nickname, const Vector& position,
                                                                              float unknown2, float unknown3, wchar_t* defaultText);
        using SetupTextEditControlFunction = void(__thiscall*)(FlTextEditControl* control);

        constexpr int MaxCharacterCount = 32;
        constexpr float DefaultTextEditValue1 = 0.52f;
        constexpr float DefaultTextEditValue2 = 0.03f;

    public:
        inline static auto CreateTextEditControl = reinterpret_cast<CreateTextEditControLFunction>(0x5A1900);
        inline static auto SetupTextEditControl = reinterpret_cast<SetupTextEditControlFunction>(0x57C750);
};
