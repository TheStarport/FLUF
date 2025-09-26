#pragma once
#include "FLUF.UI.hpp"

class HudManager;
class FlWireControl;
class FlToggleControl;
class FlTextEditControl;
class FlTextControl;
class FlStaticControl;
class FlShipModelControl;
class FlScrollControl;
class FlCycleControl;
class Vector;
class FlControl;
class FlButtonControl;
class AbstractVanillaInterface
{
        friend HudManager;

        using CreateFlButtonFunction = FlButtonControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* mesh, const char* mesh2,
                                                                     const char* clickSound, const char* event, const Vector& position, const char* hardpoint,
                                                                     const Vector& mouseSize, bool noMeshRender, bool actionButton, bool noColourChange,
                                                                     bool noZEnable, unsigned int tip, unsigned int tip2, unsigned int keyId);

        using CreateFlCycleControlFunction = FlCycleControl*(__thiscall*)(const FlControl* parent, const char* nickname, const Vector& position,
                                                                          float distanceAwayFromEachOther, float scale, float u1, float u2, float u3,
                                                                          bool rotateHorizontal);

        using CreateFlScrollControlFunction = FlScrollControl*(__thiscall*)(const FlControl* parent, const char* desc, const Vector& pos, float ux, float uy,
                                                                            float uz, unsigned int capacity, unsigned int count, bool horizontal);

        using CreateShipModelControlFunction = FlShipModelControl*(__thiscall*)(const FlControl* parent, const char* nickname, const Vector& automatedRotation,
                                                                                const Vector& startPosition, const Vector& startingRotation,
                                                                                const Vector& clipping, unsigned int u1, unsigned int u2, unsigned int u3,
                                                                                unsigned int& u4, unsigned int& u5);

        using CreateFlStaticControlFunction = FlStaticControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* mesh,
                                                                            const Vector& position, const char* hardpoint, const Vector& mouseSize,
                                                                            unsigned int u1, unsigned int u2, unsigned int u3, unsigned int u4, unsigned int u5,
                                                                            unsigned int u6, unsigned int u7, unsigned int u8, unsigned int u9,
                                                                            unsigned int u10, const Vector& unknownVector);

        using CreateFlTextControlFunction = FlTextControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* font, const Vector& position,
                                                                        const char* hardpoint, float sizeX, float sizeY, float sizeZ, unsigned int resStrId,
                                                                        int align, unsigned int tip, bool center);

        using CreateTextEditControLFunction = FlTextEditControl*(__thiscall*)(const FlControl* parent, const char* nickname, const Vector& position,
                                                                              float unknown2, float unknown3, wchar_t* defaultText);

        using CreateToggleControlFunction = FlToggleControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* enabled,
                                                                          const char* disabled, const char* sound, unsigned int u1, const Vector& position,
                                                                          const char* hardpoint, const Vector& mouseSize, unsigned int u2, unsigned int u3,
                                                                          unsigned int u4, unsigned int u5, unsigned int enabledResStrId,
                                                                          unsigned int disabledResStrId);

        using CreateWireControlFunc = FlWireControl*(__thiscall*)(const FlControl* parent, const char* nickname, const char* sound, const Vector& position,
                                                                  const Vector& dimensions, const char* hardpoint, unsigned int u2, unsigned int resHintId,
                                                                  unsigned int u4, unsigned int u5);

    protected:
        void FLUF_UI_API OpenDialogue(unsigned int captionIds, unsigned int bodyIds);
        void FLUF_UI_API CloseDialogue();
        virtual void OnDraw(FlControl* control) {}
        virtual void OnUpdate(FlControl* control) {}
        virtual bool OnExecuteButton(FlButtonControl* control) { return true; }
        virtual bool OnExecuteText(FlTextControl* control) { return true; }
        virtual void OnExecuteScroll(FlScrollControl* control) {}
        virtual void OnExecuteCycle(FlCycleControl* control) {}
        virtual void OnTerminate(FlControl* control) {}
        virtual ~AbstractVanillaInterface() = default;

    public:
        const CreateFlButtonFunction CreateButtonControl = reinterpret_cast<CreateFlButtonFunction>(0x5A0C90);                 // NOLINT(*-misplaced-const)
        const CreateFlCycleControlFunction CreateCycleControl = reinterpret_cast<CreateFlCycleControlFunction>(0x5A1870);      // NOLINT(*-misplaced-const)
        const CreateFlScrollControlFunction CreateScrollControl = reinterpret_cast<CreateFlScrollControlFunction>(0x5A17E0);   // NOLINT(*-misplaced-const)
        const CreateShipModelControlFunction CreateShipControl = reinterpret_cast<CreateShipModelControlFunction>(0x5A1020);   // NOLINT(*-misplaced-const)
        const CreateFlStaticControlFunction CreateStaticControl = reinterpret_cast<CreateFlStaticControlFunction>(0x5A0DB0);   // NOLINT(*-misplaced-const)
        const CreateFlTextControlFunction CreateTextControl = reinterpret_cast<CreateFlTextControlFunction>(0x5A0E30);         // NOLINT(*-misplaced-const)
        const CreateTextEditControLFunction CreateTextEditControl = reinterpret_cast<CreateTextEditControLFunction>(0x5A1900); // NOLINT(*-misplaced-const)
        const CreateToggleControlFunction CreateToggleControl = reinterpret_cast<CreateToggleControlFunction>(0x5A0FB0);       // NOLINT(*-misplaced-const)
        const CreateWireControlFunc CreateWireControl = reinterpret_cast<CreateWireControlFunc>(0x5A0D00);                     // NOLINT(*-misplaced-const)

        FLUF_UI_API FlStaticControl* CreateDefaultStaticControl(const FlControl* parent, const char* name, const char* mesh, const Vector& pos,
                                                                const char* hardpoint, const Vector& mouseSize) const;
};
