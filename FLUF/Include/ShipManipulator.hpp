#pragma once

#include <ImportFluf.hpp>

class FLUF_API ShipManipulator final
{
    public:
        static void SetAngularVelocity(CObject* object, const Vector& newVelocity);
        static void SetVelocity(CObject* object, const Vector& newVelocity);
        static void SetPosition(CObject* object, const Vector& pos);
        static Vector GetAngularVelocity(CObject* ship);
        static Vector GetVelocity(CObject* ship);
};
