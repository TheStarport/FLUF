#pragma once
#include "Structures.hpp"

class FlShipModelControl final : public FlControl
{
        byte data[1074]{};
        FlShipModelControl() = default;

        using LoadShipFunction = bool(__thiscall*)(FlShipModelControl* ship, unsigned int& arch, bool b1, bool b2);

    public:
        Vector GetShipViewMatrix() { return *reinterpret_cast<Vector*>(reinterpret_cast<unsigned int>(this) + 0x360); }
        void SetShipViewMatrix(const Vector& V) { *reinterpret_cast<Vector*>(reinterpret_cast<unsigned int>(this) + 0x360) = V; }

        const auto LoadShip = reinterpret_cast<LoadShipFunction>(0x585A90);
};
