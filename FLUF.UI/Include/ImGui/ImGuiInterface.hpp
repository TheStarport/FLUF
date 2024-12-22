#pragma once

#include "FlufUiConfig.hpp"
#include "ImGuiModule.hpp"

#include <unordered_set>

class FlufUi;
class ImGuiInterface
{
        struct MouseState
        {
                bool leftDown;
                bool rightDown;
                bool middleDown;
                bool mouse4Down;
                bool mouse5Down;
        };

        friend FlufUi;

        std::shared_ptr<FlufUiConfig> config;
        RenderingBackend backend;

        void Render(const std::unordered_set<ImGuiModule*>& imguiModules) const;
        static void PollInput();
        static MouseState ConvertState(DWORD state);
        static bool WndProc(FlufUiConfig* config, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    public:
        ~ImGuiInterface();
        explicit ImGuiInterface(FlufUi* flufUi, RenderingBackend backend, void* device);
};
