#pragma once

#include "RmlUi/Core/Input.h"
#include "RmlUi/Core/RenderInterface.h"

#include <Rml/RmlContext.hpp>

#include <FLCore/FLCoreDefs.hpp>
#include <Utils/Detour.hpp>

class FlufUi;
class SystemInterface;
class FileInterface;

class RmlInterface
{
        friend FlufUi;

        bool WndProc(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);
        static bool WinKey(uint msg, WPARAM wParam, LPARAM lParam);
        void Render();

        void PollMouse();
        void PollKeyboard();
        void PollInput();

        void LoadFonts();

        bool shutDown = false;
        std::unordered_set<std::string> fonts;
        FlufUi* ui;
        std::array<bool, 5> lastMouseState{};
        std::array<bool, 5> currentMouseState{};
        std::unique_ptr<Rml::RenderInterface> renderInterface;
        std::unique_ptr<SystemInterface> systemInterface;
        std::unique_ptr<FileInterface> fileInterface;
        inline static Rml::Context* rmlContext;

    public:
        explicit RmlInterface(FlufUi* fluf, IDirect3D9* d3d9, IDirect3DDevice9* device);
        ~RmlInterface();

        static RmlContext GetRmlContext();
};
