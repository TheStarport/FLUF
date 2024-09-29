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
class FlufUi
{
        std::shared_ptr<HudManager> hudManager;

        void DelayedInit();
        static void* OnScriptLoadHook(const char* file);
        static IDirect3D9* __stdcall OnDirect3D8Create(uint sdkVersion);
        static HRESULT __stdcall OnDirect3D9CreateDevice(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                         D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);

    public:
        API static std::weak_ptr<FlufUi> Instance();
        API std::weak_ptr<HudManager> GetHudManager();

        FlufUi();
        ~FlufUi();
};
