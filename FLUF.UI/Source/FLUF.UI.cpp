#include "PCH.hpp"

#include "FLUF.UI.hpp"

#include "Rml/Interfaces/RenderInterfaceDirectX9.hpp"
#include "Typedefs.hpp"
#include "Utils/Detour.hpp"
#include "Vanilla/HudManager.hpp"

#include <d3dx9.h>

using ScriptLoadPtr = void* (*)(const char* fileName);
using Direct3DCreate8Ptr = IDirect3D9*(__stdcall*)(uint sdkVersion);
using Direct3DCreateDevice9 = HRESULT(__stdcall*)(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                  D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);

std::shared_ptr<FlufUi> module;

std::unique_ptr<FunctionDetour<ScriptLoadPtr>> thornLoadDetour;
std::unique_ptr<FunctionDetour<Direct3DCreate8Ptr>> d3d8CreateDetour;
std::unique_ptr<FunctionDetour<Direct3DCreateDevice9>> d3d8CreateDeviceDetour;

// ReSharper disable twice CppUseAuto
st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    if (reason == DLL_PROCESS_ATTACH)
    {
        module = std::make_shared<FlufUi>();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        module.reset();
    }

    return TRUE;
}

void FlufUi::DelayedInit() {}

void* FlufUi::OnScriptLoadHook(const char* file)
{
    static bool loaded = false;
    if (!loaded)
    {
        loaded = true;
        module->DelayedInit();
    }

    thornLoadDetour->UnDetour();
    void* ret = thornLoadDetour->GetOriginalFunc()(file);
    thornLoadDetour->Detour(OnScriptLoadHook);
    return ret;
}

IDirect3D9* __stdcall FlufUi::OnDirect3D8Create(const uint sdkVersion)
{
    d3d8CreateDetour->UnDetour();
    IDirect3D9* d3d9 = d3d8CreateDetour->GetOriginalFunc()(sdkVersion);

    if (HMODULE dll; RtlPcToFileHeader(d3d9, reinterpret_cast<void**>(&dll)))
    {
        // If successful, prepend
        if (CHAR maxPath[MAX_PATH]; GetModuleFileNameA(dll, maxPath, MAX_PATH))
        {
            const std::string path = maxPath;
            if (const std::string file = path.substr(path.find_last_of('\\') + 1); file != "d3d9.dll")
            {
                // TODO: Log
                MessageBoxA(nullptr, "This isn't DirectX9.", "Bad renderer", MB_OK);

                // return without hooking, they are not running DirectX9
                return d3d9;
            }
        }
    }

    const auto vtable = reinterpret_cast<DWORD**>(*reinterpret_cast<DWORD*>(d3d9));
    d3d8CreateDeviceDetour = std::make_unique<FunctionDetour<Direct3DCreateDevice9>>(reinterpret_cast<Direct3DCreateDevice9>(*vtable[17]));
    d3d8CreateDeviceDetour->Detour(OnDirect3D9CreateDevice);

    return d3d9;
}

HRESULT __stdcall FlufUi::OnDirect3D9CreateDevice(IDirect3D9* context, const uint adapter, const D3DDEVTYPE deviceType, const HWND focusWindow,
                                                  const DWORD behaviorFlags, D3DPRESENT_PARAMETERS* presentationParameters,
                                                  IDirect3DDevice9** returnedDeviceInterface)
{
    d3d8CreateDeviceDetour->UnDetour();
    const auto result =
        d3d8CreateDeviceDetour->GetOriginalFunc()(context, adapter, deviceType, focusWindow, behaviorFlags, presentationParameters, returnedDeviceInterface);

    d3d8CreateDeviceDetour->Detour(OnDirect3D9CreateDevice);
    return result;
}

std::weak_ptr<FlufUi> FlufUi::Instance() { return module; }
std::weak_ptr<HudManager> FlufUi::GetHudManager() { return hudManager; }

FlufUi::FlufUi()
{
    hudManager = std::make_shared<HudManager>();

    const HMODULE common = GetModuleHandleA("common");
    thornLoadDetour = std::make_unique<FunctionDetour<ScriptLoadPtr>>(
        reinterpret_cast<ScriptLoadPtr>(GetProcAddress(common, "?ThornScriptLoad@@YAPAUIScriptEngine@@PBD@Z"))); // NOLINT

    thornLoadDetour->Detour(OnScriptLoadHook);

    const HMODULE d3d8 = GetModuleHandleA("d3d8.dll");
    d3d8CreateDetour = std::make_unique<FunctionDetour<Direct3DCreate8Ptr>>(reinterpret_cast<Direct3DCreate8Ptr>(GetProcAddress(d3d8, "Direct3DCreate8")));
    d3d8CreateDetour->Detour(OnDirect3D8Create);
}

FlufUi::~FlufUi() { hudManager.reset(); }
