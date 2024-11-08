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

        using OnUiRender = bool (*)();

        void PollMouse();
        void PollKeyboard();
        void PollInput();

        void LoadFonts();
        static LRESULT __stdcall WndProc(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);
        static bool WinKeyDetour(const uint msg, const WPARAM wParam, const LPARAM lParam);
        static bool UiRenderDetour();

        bool shutDown = false;
        std::unordered_set<std::string> fonts;
        FlufUi* ui;
        std::array<bool, 5> lastMouseState{};
        std::array<bool, 5> currentMouseState{};
        std::unique_ptr<Rml::RenderInterface> renderInterface;
        std::unique_ptr<SystemInterface> systemInterface;
        std::unique_ptr<FileInterface> fileInterface;
        inline static Rml::Context* rmlContext;
        inline static FunctionDetour<OnUiRender> uiRenderDetour{ reinterpret_cast<OnUiRender>(0x41F150) };

    public:
        explicit RmlInterface(FlufUi* fluf, IDirect3D9* d3d9, IDirect3DDevice9* device);
        ~RmlInterface();

        static RmlContext GetRmlContext();
};
