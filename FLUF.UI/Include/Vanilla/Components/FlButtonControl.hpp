#pragma once

#include "FLCore/FLCoreCommon.h"
#include "Structures.hpp"

class FlButtonControl final : public FlControl
{
        byte data[888]{}; // 0x378 in sub_55D2B0
        FlButtonControl() = default;

        using FlButtonFunction = FlButtonControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* mesh, const char* mesh2,
                                                               const char* clickSound, const char* event, const Vector& position, const char* hardpoint,
                                                               const Vector& mouseSize, bool noMeshRender, bool actionButton, bool noColourChange,
                                                               bool noZEnable, uint tip, uint tip2, uint keyId);

    public:
        inline static auto CreateButtonControl = reinterpret_cast<FlButtonFunction>(0x5A0C90);
};
