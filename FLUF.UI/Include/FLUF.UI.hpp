#pragma once

#include "ImportFlufUi.hpp"

#include "FlufUiConfig.hpp"

#include <Rml/RmlContext.hpp>
#include <d3d9types.h>

class HudManager;
class IDirect3D9;
class IDirect3DDevice9;
class RmlInterface;
class FlufUi
{
        inline static IDirect3D9* d3d9;
        inline static IDirect3DDevice9* d3d9device;

        std::shared_ptr<HudManager> hudManager;
        std::shared_ptr<RmlInterface> rmlInterface;
        std::shared_ptr<FlufUiConfig> config;

        void DelayedInit();

        static void OnUpdate(double delta);
        static void* OnScriptLoadHook(const char* file);
        static IDirect3D9* __stdcall OnDirect3D8Create(uint sdkVersion);
        static HRESULT __stdcall OnDirect3D9CreateDevice(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                         D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);
        static HRESULT __stdcall OnDirect3D9EndScene(IDirect3DDevice9* device);

    public:
        FLUF_UI_API static std::weak_ptr<FlufUi> Instance();
        FLUF_UI_API std::weak_ptr<HudManager> GetHudManager();
        FLUF_UI_API std::optional<RmlContext> GetRmlContext();
        FLUF_UI_API std::shared_ptr<FlufUiConfig> GetConfig();

        FlufUi();
        ~FlufUi();
};
