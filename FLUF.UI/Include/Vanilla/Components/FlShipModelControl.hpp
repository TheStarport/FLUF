#pragma once
#include "Structures.hpp"

class FlShipModelControl final : public FlControl
{
        byte data[1074]{};
        FlShipModelControl() = default;

        using CreateShipModelControlFunction = FlShipModelControl*(__thiscall*)(const FlControl* parent, const char* nickname, const Vector& automatedRotation,
                                                                                const Vector& startPosition, const Vector& startingRotation,
                                                                                const Vector& clipping, uint u1, uint u2, uint u3, uint& u4, uint& u5);
        using LoadShipFunction = bool(__thiscall*)(FlShipModelControl* ship, uint& arch, bool b1, bool b2);

    public:
        Vector GetShipViewMatrix() { return *reinterpret_cast<Vector*>(reinterpret_cast<uint>(this) + 0x360); }
        void SetShipViewMatrix(const Vector& V) { *reinterpret_cast<Vector*>(reinterpret_cast<uint>(this) + 0x360) = V; }

        inline static auto CreateShipControl = reinterpret_cast<CreateShipModelControlFunction>(0x5A1020);
        inline static auto LoadShip = reinterpret_cast<LoadShipFunction>(0x585A90);
};
