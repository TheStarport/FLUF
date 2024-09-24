#pragma once
#include "Structures.hpp"

class FlStaticControl final : public FlControl
{
        byte data[1056]{}; // 0x420 in sub_589A20
        FlStaticControl() = default;

        using CreateFlStaticControlFunction = FlStaticControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* mesh,
                                                                            const Vector& position, const char* hardpoint, const Vector& mouseSize, uint u1,
                                                                            uint u2, uint u3, uint u4, uint u5, uint u6, uint u7, uint u8, uint u9, uint u10,
                                                                            const Vector& unknownVector);

    public:
        inline static auto CreateStaticControl = reinterpret_cast<CreateFlStaticControlFunction>(0x5A0DB0);
        static FlStaticControl* CreateDefaultStaticControl(const FlControl* parent, const char* name, const char* mesh, const Vector& pos,
                                                           const char* hardpoint, const Vector& mouseSize)
        {
            return CreateStaticControl(parent, name, mesh, pos, hardpoint, mouseSize, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, { 0, 0, 0 });
        }
};
