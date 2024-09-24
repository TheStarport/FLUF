#pragma once
#include "Structures.hpp"

class FlTextControl : public FlControl
{
        byte data[928]{}; // 0x3A0 in sub_58C120

        using CreateFlTextControlFunction = FlTextControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* font, const Vector& position,
                                                                        const char* hardpoint, float sizeX, float sizeY, float sizeZ, uint resStrId, int align,
                                                                        uint tip, bool center);

    protected:
        FlTextControl() = default;

    public:
        Color4 GetColor() { return *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x348); }
        void SetColor(const Color4& C) { *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x348) = C; }
        bool GetInteractable() { return *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x35C); }
        void SetInteractable(bool value) { *reinterpret_cast<bool*>(reinterpret_cast<uint>(this) + 0x35C) = value; }
        Color4 GetInteractableColorUse() { return *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x35D); }
        void SetInteractableColorUse(const Color4& value) { *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x35D) = value; }
        Color4 GetColorHighlighted() { return *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x354); }
        void SetColourHighlighted(const Color4& value) { *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x354) = value; }
        Color4 GetInteractableColorValue() { return *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x35E); }
        void SetInteractableColourValue(const Color4 c) { *reinterpret_cast<Color4*>(reinterpret_cast<uint>(this) + 0x35E) = c; }

        // bool HasText() { return static_cast<bool>(Perform(0x21, 0, 0)); }

        void SetTextValue(const wchar_t* message) { Perform(0x1D, reinterpret_cast<uint>(message), 0); }
        std::wstring_view GetTextValue()
        {
            // if (HasText())
            auto test = reinterpret_cast<wchar_t*>(Perform(0x1F, 0, 0));
            if (!test)
            {
                return L"";
            }

            // ReSharper disable once CppDFALocalValueEscapesFunction
            return { test };
        }

        inline static auto CreateTextControl = reinterpret_cast<CreateFlTextControlFunction>(0x5A0E30);
};
