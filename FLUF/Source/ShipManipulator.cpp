#include <PCH.hpp>

#include "ShipManipulator.hpp"

void ShipManipulator::SetAngularVelocity(CObject* object, const Vector& newVelocity)
{
    const auto v = Vector(newVelocity.x, newVelocity.y, newVelocity.z);
    const uint physicsObject = *reinterpret_cast<uint*>(reinterpret_cast<PCHAR>(*reinterpret_cast<uint*>(reinterpret_cast<uint>(object) + 84)) + 152);
    if (!physicsObject)
    {
        return;
    }
    *reinterpret_cast<Vector*>(physicsObject + 148) = v;
}

Vector ShipManipulator::GetAngularVelocity(CObject* ship)
{
    const uint physicsObject = *reinterpret_cast<uint*>(reinterpret_cast<PCHAR>(*reinterpret_cast<uint*>(reinterpret_cast<uint>(ship) + 84)) + 152);
    if (!physicsObject)
    {
        return {};
    }

    return *reinterpret_cast<Vector*>(physicsObject + 148);
}

Vector ShipManipulator::GetVelocity(CObject* ship)
{
    const uint physicsObject = *reinterpret_cast<uint*>(reinterpret_cast<PCHAR>(*reinterpret_cast<uint*>(reinterpret_cast<uint>(ship) + 84)) + 152);
    if (!physicsObject)
    {
        return {};
    }

    return *reinterpret_cast<Vector*>(physicsObject + 164);
}

void ShipManipulator::SetVelocity(CObject* object, const Vector& newVelocity)
{
    const auto v = Vector(newVelocity.x, newVelocity.y, newVelocity.z);
    const uint physicsObject = *reinterpret_cast<uint*>(PCHAR(*reinterpret_cast<uint*>(uint(object) + 84)) + 152);
    if (!physicsObject)
    {
        return;
    }

    *reinterpret_cast<Vector*>(physicsObject + 164) = v;
}

void ShipManipulator::SetPosition(CObject* object, const Vector& pos)
{
    const auto v = Vector(pos.x, pos.y, pos.z);
    *reinterpret_cast<Vector*>(reinterpret_cast<PCHAR>(object) + 44) = v;
    object->update_tree();
}
