#pragma once

#include "RmlUi/Core/Input.h"
#include "RmlUi/Core/RenderInterface.h"

#include <dinput.h>

class FlufUi;
class SystemInterface;
class FileInterface;

class RmlInterface
{
        static bool HasKeyboardModifer(int modifiers, Rml::Input::KeyModifier modifier);
        static int GetKeyboardModifiers(BYTE diKeys[]);
        static bool KeyboardButtonDown(BYTE diKeys[], BYTE key);

        void PollDInput8();
        void LoadFonts();

        inline static IDirectInput8* pDirectInput;
        inline static LPDIRECTINPUTDEVICE8 lpdiKeyboard;

        std::unordered_set<std::string> fonts;
        FlufUi* ui;

        std::unique_ptr<Rml::RenderInterface> renderInterface;
        std::unique_ptr<SystemInterface> systemInterface;
        std::unique_ptr<FileInterface> fileInterface;

    public:
        explicit RmlInterface(FlufUi* fluf);
        ~RmlInterface();
};
