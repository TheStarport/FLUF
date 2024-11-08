#pragma once

#include <FLCore/FLCoreDefs.hpp>
#include <functional>
#include <string>

#pragma warning(push)
#pragma warning(disable : 4554)

typedef unsigned char byte;

#define EventTractor                "tractor"
#define EventNext                   "next"
#define EventPrev                   "prev"
#define EventNextSubTarget          "next_subtarget"
#define EventPrevSubTarget          "prev_subtarget"
#define EventNull                   ""

#define PerformToggleOff            Perform(0x30, 0, 0)
#define PerformToggleOn             Perform(0x30, 1, 0)

#define PerformButtonOff            Perform(0xC, 0, 0)
#define PerformButtonOn             Perform(0xC, 1, 0)

#define PerformSliderSetCapacity(x) Perform(0x37, 1, x)
#define PerformSliderSetPosition(x) Perform(0x38, x, 0)

class FlControl
{
    public:
        virtual ~FlControl() = default;
        virtual void u00();
        virtual void u04();
        virtual void IO_streaminit();
        virtual void u0C();
        virtual void u10();
        virtual void u14();
        virtual void u18();
        virtual void u1C();
        virtual void u20();
        virtual void u24();
        virtual void u28();
        virtual void u2C();
        virtual void u30();
        virtual void Draw();
        virtual void u38();
        virtual void u3C();
        virtual void u40();
        virtual void u44();
        virtual void u48();
        virtual void u4C();
        virtual void u50();
        virtual void u54();
        virtual void u58();
        virtual void u5C();
        virtual void u60();
        virtual void u64();
        virtual void u68();
        virtual void u6C();
        virtual void u70();
        virtual void u74();
        virtual void u78();
        virtual void u7C();
        virtual void u80();
        virtual void u84();
        virtual void u88();
        virtual void u8C();
        virtual void u90();
        virtual void u94();
        virtual void u98();
        virtual void u9C();
        virtual void uA0();
        virtual void uA4();
        virtual int Perform(int Message, unsigned int LPARAM, unsigned int WPARAM);
        // byte dunno[0xAC];

        [[nodiscard]]
        std::string_view GetName() const
        {
            return { reinterpret_cast<char*>(reinterpret_cast<unsigned int>(this) + 0x18) };
        }

        [[nodiscard]]
        bool GetEnabled() const
        {
            const byte value = *reinterpret_cast<byte*>(reinterpret_cast<unsigned int>(this) + 0x6C);
            return (value & 0x08) == 0x08;
        }

        void SetEnabled(const bool state) const
        {
            byte value = *reinterpret_cast<byte*>(reinterpret_cast<unsigned int>(this) + 0x6C);
            const byte part1 = value & 0x07;

            byte part2;
            if (state)
            {
                part2 = 0x08;
            }
            else
            {
                part2 = 0x00;
            }
            value = part1 | part2;
            *reinterpret_cast<byte*>(reinterpret_cast<unsigned int>(this) + 0x6C) = value;
        }

        [[nodiscard]]
        bool GetVisible() const
        {
            const byte value = *reinterpret_cast<byte*>(reinterpret_cast<unsigned int>(this) + 0x6C);
            // ReSharper disable once CppIdenticalOperandsInBinaryExpression
            return (value & 0x03) == 0x03;
        }

        void SetVisible(const bool newstate) const
        {
            byte currentstate = *reinterpret_cast<byte*>(reinterpret_cast<unsigned int>(this) + 0x6C);
            const byte part1 = currentstate & 0xFD;

            byte part2;
            if (newstate != 0)
            {
                part2 = 0x03;
            }
            else
            {
                part2 = 0x00;
            }
            currentstate = part1 | part2;
            *reinterpret_cast<byte*>(reinterpret_cast<unsigned int>(this) + 0x6C) = currentstate;
        }

        [[nodiscard]]
        Vector GetPosition() const
        {
            return *reinterpret_cast<Vector*>(reinterpret_cast<unsigned int>(this) + 0xA8);
        }

        void SetPosition(const Vector& p) const { *reinterpret_cast<Vector*>(reinterpret_cast<unsigned int>(this) + 0xA8) = p; }

        [[nodiscard]]
        FlControl* GetParentControl() const
        {
            return reinterpret_cast<FlControl*>(*reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned long>(this) + 0x04));
        }

        [[nodiscard]]
        FlControl* GetNextControl() const
        {
            return reinterpret_cast<FlControl*>(*reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned long>(this) + 0x08));
        }

        [[nodiscard]]
        FlControl* GetChildControl() const
        {
            return reinterpret_cast<FlControl*>(*reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned long>(this) + 0x0C));
        }

        void ForEachControl(const std::function<void(FlControl*)>& action, const bool siblings = false)
        {
            action(this);

            FlControl* child = this->GetChildControl();
            FlControl* prev = nullptr;
            while (child)
            {
                if (child == prev)
                {
                    break;
                }

                child->ForEachControl(action, true);
                prev = child;
                child = child->GetNextControl();
            }

            if (siblings)
            {
                FlControl* next;
                while ((next = this->GetNextControl()))
                {
                    if (next == prev)
                    {
                        break;
                    }

                    next->ForEachControl(action, false);
                    prev = next;
                }
            }
        }

        bool ControlExists(FlControl* control, const bool siblings = true)
        {
            if (reinterpret_cast<unsigned long>(this) == reinterpret_cast<unsigned long>(control))
            {
                return true;
            }

            FlControl* prev = nullptr;
            FlControl* child = this->GetChildControl();
            while (child)
            {
                if (child == prev)
                {
                    break;
                }

                auto found = child->ControlExists(control, true);
                if (found)
                {
                    return found;
                }

                prev = child;
                child = child->GetNextControl();
            }

            if (siblings)
            {
                FlControl* next;
                while ((next = this->GetNextControl()))
                {
                    if (next == prev)
                    {
                        break;
                    }

                    if (next->ControlExists(control, false))
                    {
                        return next;
                    }

                    prev = next;
                }
            }

            // Not found
            return false;
        }

        FlControl* FindControlByName(const std::string& name)
        {
            if (this->GetName() == name)
            {
                return this;
            }

            FlControl* child = FindChildControl(name);
            if (child)
            {
                return child;
            }

            FlControl* next;
            while ((next = this->GetNextControl()))
            {
                if (next->GetName() == name)
                {
                    return next;
                }

                child = FindChildControl(name);
                if (child)
                {
                    return child;
                }
            }

            // Not found
            return nullptr;
        }

        [[nodiscard]]
        FlControl* FindChildControl(const std::string& name) const // NOLINT(*-no-recursion)
        {
            FlControl* child = this->GetChildControl();
            while (child)
            {
                if (child->GetName() == name)
                {
                    return child;
                }

                if (child->GetChildControl() != nullptr)
                {
                    if (FlControl* newChild = child->FindChildControl(name); newChild != nullptr)
                    {
                        return newChild;
                    }
                }

                child = child->GetNextControl();
            }

            // Not found
            return nullptr;
        }

        void SetParentControl(FlControl* control)
        {
            const unsigned int data = reinterpret_cast<unsigned int>(this) + 0x04;
            *reinterpret_cast<FlControl**>(data) = control;
        }

        void SetNextControl(FlControl* control)
        {
            const unsigned int data = reinterpret_cast<unsigned int>(this) + 0x08;
            *reinterpret_cast<FlControl**>(data) = control;
        }

        void SetChildControl(FlControl* control)
        {
            const unsigned int data = reinterpret_cast<unsigned int>(this) + 0x0C;
            *reinterpret_cast<FlControl**>(data) = control;
        }
};

struct Color4
{
        byte R;
        byte G;
        byte B;
        byte A;
};

enum class Location
{
    Left,
    Center,
    Right
};
