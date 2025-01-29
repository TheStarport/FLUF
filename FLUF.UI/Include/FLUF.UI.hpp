#pragma once

#include "ImportFlufUi.hpp"

#include "FlufModule.hpp"
#include "FlufUiConfig.hpp"
#include "ImGui/ImGuiModule.hpp"

#include "Utils/Detour.hpp"

#include <d3d9types.h>

class ImGuiInterface;
class HudManager;
class IDirect3D9;
class IDirect3DDevice9;

/**
 * @author Laz
 * @brief
 * FLUF UI offers a number of different methods for manipulating / adding to the Freelancer GUI.
 *
 * At the time of writing, there are two methods. 'Vanilla' style and 'ImGui' style (these can be used together).
 * Vanilla style lets you remove, trigger, listen to, or add elements to the original Freelancer interface.
 * ImGui is one of the most used UI libraries and is very easy to use, but somewhat difficult to style effectively.
 * It requires DirectX9 in order to run, currently this can be done by using d3d8to9.
 */
class FlufUi final : public FlufModule
{
        friend ImGuiInterface;
        inline static IDirect3D9* d3d9;
        inline static IDirect3DDevice9* d3d9device;
        using OnUiRender = bool (*)();

        using WinKeyType = bool (*)(uint msg, WPARAM wParam, LPARAM lParam);
        using OriginalWndProc = LRESULT(__stdcall*)(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);

        FunctionDetour<WinKeyType> winKeyDetour{ reinterpret_cast<WinKeyType>(0x577850) };
        FunctionDetour<OriginalWndProc> wndProcDetour{ reinterpret_cast<OriginalWndProc>(0x5B2570) };
        FunctionDetour<OnUiRender> uiRenderDetour{ reinterpret_cast<OnUiRender>(0x41F150) };

        std::shared_ptr<HudManager> hudManager;
        std::shared_ptr<FlufUiConfig> config;
        std::shared_ptr<ImGuiInterface> imguiInterface;

        /**
         * @brief Hook on OnGameLoad (main menu loaded). Initialises RML with Directx9 if the mode is enabled and DX9 is found.
         */
        void OnGameLoad() override;

        static LRESULT __stdcall WndProc(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);
        static bool WinKeyDetour(uint msg, WPARAM wParam, LPARAM lParam);
        static bool UiRenderDetour();
        static IDirect3D9* __stdcall OnDirect3D8Create(uint sdkVersion);
        static HRESULT __stdcall OnDirect3D9ResetDevice(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
        static HRESULT __stdcall OnDirect3D9CreateDevice(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                         D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);
        bool OpenOptionsMenu() const;

    public:
        static constexpr std::string_view moduleName = "FLUF.UI";

        /**
         * @brief Gets the HudManager class which is used to manipulate the vanilla freelancer interface.
         */
        FLUF_UI_API std::weak_ptr<HudManager> GetHudManager();

        /**
         * @brief Gets the config settings for FLUF UI
         */
        FLUF_UI_API std::shared_ptr<FlufUiConfig> GetConfig();

        /**
         * @brief Gets the ImGui interface for loading textures or registering modules. Will be a nullptr if the current ui mode is not set to imgui.
         */
        [[nodiscard]]
        FLUF_UI_API ImGuiInterface* GetImGuiInterface() const;

        FlufUi();
        ~FlufUi() override;
        FLUF_UI_API std::string_view GetModuleName() override;
};
