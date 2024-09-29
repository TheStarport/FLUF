#pragma once

#ifdef FLUF_UI
    #define API __declspec(dllexport)
#else
    #define API __declspec(dllimport)
#endif

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

        void DelayedInit();

        static void OnUpdate(double delta);
        static void* OnScriptLoadHook(const char* file);
        static IDirect3D9* __stdcall OnDirect3D8Create(uint sdkVersion);
        static HRESULT __stdcall OnDirect3D9CreateDevice(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                         D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);
        static HRESULT __stdcall OnDirect3D9EndScene(IDirect3DDevice9* device);

    public:
        API static std::weak_ptr<FlufUi> Instance();
        API std::weak_ptr<HudManager> GetHudManager();
        API std::weak_ptr<RmlInterface> GetRmlInterface();

        FlufUi();
        ~FlufUi();
};
