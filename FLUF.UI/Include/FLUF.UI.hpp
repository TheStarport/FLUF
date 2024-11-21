#pragma once

#include "ImportFlufUi.hpp"

#include "FLUF/Include/FlufModule.hpp"
#include "FlufUiConfig.hpp"

#include <Rml/RmlContext.hpp>
#include <d3d9types.h>

class HudManager;
class IDirect3D9;
class IDirect3DDevice9;
class RmlInterface;

/**
 * @author Laz
 * @brief
 * FLUF UI offers a number of different methods for manipulating / adding to the Freelancer GUI.
 *
 * At the time of writing, there are two methods. 'Vanilla' style, and 'Rml' style (these can be used together).
 * Vanilla style lets you remove, trigger, listen to, or add elements to the original Freelancer interface.
 * Rml uses a HTML/CSS based solution using the [RmlUi](https://github.com/mikke89/RmlUi) library.
 * It requires DirectX9 in order to run, currently this can be done by using d3d8to9.
 */
class FlufUi final : public FlufModule
{
        inline static IDirect3D9* d3d9;
        inline static IDirect3DDevice9* d3d9device;

        std::shared_ptr<HudManager> hudManager;
        std::shared_ptr<RmlInterface> rmlInterface;
        std::shared_ptr<FlufUiConfig> config;

        /**
         * @brief Hook on OnGameLoad (main menu loaded). Initialises RML with Directx9 if the mode is enabled and DX9 is found.
         */
        void OnGameLoad() override;

        static IDirect3D9* __stdcall OnDirect3D8Create(uint sdkVersion);
        static HRESULT __stdcall OnDirect3D9CreateDevice(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                         D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);

    public:
        static constexpr std::string_view moduleName = "FLUF.UI";

        /**
         * @brief Gets the HudManager class which is used to manipulate the vanilla freelancer interface.
         */
        FLUF_UI_API std::weak_ptr<HudManager> GetHudManager();

        /**
         * @brief Gets the RmlContext for doing basic document and data model management. Gives access to the full context if needed.
         */
        FLUF_UI_API std::optional<RmlContext> GetRmlContext();

        /**
         * @brief Gets the config settings for FLUF UI
         */
        FLUF_UI_API std::shared_ptr<FlufUiConfig> GetConfig();

        FlufUi();
        ~FlufUi() override;
        FLUF_UI_API std::string_view GetModuleName() override;
};
