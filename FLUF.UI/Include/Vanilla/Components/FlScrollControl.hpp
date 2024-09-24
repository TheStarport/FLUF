#pragma once
#include "Structures.hpp"

class FlScrollControl final : public FlControl
{
        byte data[1072] = {}; // 0x430 in sub_597AB0
        FlScrollControl() = default;
        using CreateFlScrollControlFunction = FlScrollControl*(__thiscall*)(const FlControl* parent, const char* desc, const Vector& pos, float ux, float uy,
                                                                            float uz, uint capacity, uint count, bool horizontal);

    public:
        float GetCurrentPosition() { return *reinterpret_cast<float*>(reinterpret_cast<uint>(this) + 0x3C8); }
        void SetCurrentPosition(const uint value) { Perform(0x38, value, 0); }
        void SetCurrentCapacity(const uint cap) { Perform(0x37, 1, cap); }

        inline static auto CreateScrollControl = reinterpret_cast<CreateFlScrollControlFunction>(0x5A17E0);
};
