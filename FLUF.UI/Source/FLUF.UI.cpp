#include "PCH.hpp"

#include "FLUF.UI.hpp"

#include "FLUF/Include/Fluf.hpp"
#include "Rml/Interfaces/RenderInterfaceDirectX9.hpp"
#include "Rml/Interfaces/RmlInterface.hpp"
#include "Typedefs.hpp"
#include "Utils/Detour.hpp"
#include "Vanilla/HudManager.hpp"

#include <d3dx9.h>

using ScriptLoadPtr = void* (*)(const char* fileName);
using FrameUpdatePtr = void (*)(double delta);
using Direct3DCreate9Ptr = IDirect3D9*(__stdcall*)(uint sdkVersion);
using Direct3DCreateDevice9 = HRESULT(__stdcall*)(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                  D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);
using Direct3DDevice9EndScene = HRESULT(__stdcall*)(IDirect3DDevice9* device);

std::unique_ptr<FunctionDetour<Direct3DCreate9Ptr>> d3d8CreateDetour;
std::unique_ptr<FunctionDetour<Direct3DCreateDevice9>> d3d8CreateDeviceDetour;

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void FlufUi::OnGameLoad()
{
    if (d3d9)
    {
        if (config->uiMode == UiMode::Rml)
        {
            Fluf::Log(LogLevel::Info, "Create RmlInterface");
            rmlInterface = std::make_shared<RmlInterface>(this, d3d9, d3d9device);
        }
    }
}

IDirect3D9* __stdcall FlufUi::OnDirect3D8Create(const uint sdkVersion)
{
    d3d8CreateDetour->UnDetour();
    d3d9 = d3d8CreateDetour->GetOriginalFunc()(sdkVersion);

    const auto vtable = reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(d3d9));
    d3d8CreateDeviceDetour = std::make_unique<FunctionDetour<Direct3DCreateDevice9>>(reinterpret_cast<Direct3DCreateDevice9>(vtable[16]));
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
    d3d9device = *returnedDeviceInterface;

    d3d8CreateDeviceDetour->Detour(OnDirect3D9CreateDevice);
    return result;
}

std::weak_ptr<HudManager> FlufUi::GetHudManager() { return hudManager; }
std::optional<RmlContext> FlufUi::GetRmlContext()
{
    auto context = RmlInterface::GetRmlContext();
    if (context.context)
    {
        return { context };
    };

    return std::nullopt;
}

std::shared_ptr<FlufUiConfig> FlufUi::GetConfig() { return config; }

FlufUi::FlufUi()
{
    hudManager = std::make_shared<HudManager>();
    config = std::make_shared<FlufUiConfig>();
    config->Load();

    if (const HMODULE d3d9Handle = GetModuleHandleA("d3d9.dll"))
    {
        d3d8CreateDetour =
            std::make_unique<FunctionDetour<Direct3DCreate9Ptr>>(reinterpret_cast<Direct3DCreate9Ptr>(GetProcAddress(d3d9Handle, "Direct3DCreate9")));
        d3d8CreateDetour->Detour(OnDirect3D8Create);
    }
}

FlufUi::~FlufUi() { hudManager.reset(); }

std::string_view FlufUi::GetModuleName() { return moduleName; }

SETUP_MODULE(FlufUi);
