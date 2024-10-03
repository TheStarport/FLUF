#include "PCH.hpp"

#include "FLUF.UI.hpp"

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

std::shared_ptr<FlufUi> module;

std::unique_ptr<FunctionDetour<ScriptLoadPtr>> thornLoadDetour;
std::unique_ptr<FunctionDetour<FrameUpdatePtr>> frameUpdateDetour;
std::unique_ptr<FunctionDetour<Direct3DCreate9Ptr>> d3d8CreateDetour;
std::unique_ptr<FunctionDetour<Direct3DCreate9Ptr>> d3d9CreateDetour;
std::unique_ptr<FunctionDetour<Direct3DCreateDevice9>> d3d8CreateDeviceDetour;
std::unique_ptr<FunctionDetour<Direct3DDevice9EndScene>> d3d9EndSceneDetour;

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

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

void FlufUi::DelayedInit()
{
    if (d3d9)
    {
        rmlInterface = std::make_shared<RmlInterface>(this, d3d9, d3d9device);
    }
}

void FlufUi::OnUpdate(const double delta)
{
    constexpr float SixtyFramesPerSecond = 1.0f / 60.0f;
    static double timeCounter = 0.0f;

    timeCounter += delta;
    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (timeCounter > SixtyFramesPerSecond)
    {
        // Fixed Update
        timeCounter -= SixtyFramesPerSecond;
    }

    const auto context = module->rmlInterface->GetRmlContext();
    module->rmlInterface->rmlContext->Update();

    frameUpdateDetour->UnDetour();
    frameUpdateDetour->GetOriginalFunc()(delta);
    frameUpdateDetour->Detour(OnUpdate);
}

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

    // Hook the EndScene
    const auto vtable = reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(d3d9device));
    d3d9EndSceneDetour.reset();
    d3d9EndSceneDetour = std::make_unique<FunctionDetour<Direct3DDevice9EndScene>>(reinterpret_cast<Direct3DDevice9EndScene>(vtable[42]));
    d3d9EndSceneDetour->Detour(OnDirect3D9EndScene);

    d3d8CreateDeviceDetour->Detour(OnDirect3D9CreateDevice);
    return result;
}

HRESULT __stdcall FlufUi::OnDirect3D9EndScene(IDirect3DDevice9* device)
{
    if (module->rmlInterface)
    {
        // module->rmlInterface->PollInput();
        // module->rmlInterface->rmlContext->Render();
    }

    d3d9EndSceneDetour->UnDetour();
    auto result = d3d9EndSceneDetour->GetOriginalFunc()(device);
    d3d9EndSceneDetour->Detour(OnDirect3D9EndScene);

    return result;
}

std::weak_ptr<FlufUi> FlufUi::Instance() { return module; }
std::weak_ptr<HudManager> FlufUi::GetHudManager() { return hudManager; }
std::weak_ptr<RmlInterface> FlufUi::GetRmlInterface() { return rmlInterface; }

FlufUi::FlufUi()
{
    hudManager = std::make_shared<HudManager>();

    const HMODULE common = GetModuleHandleA("common");
    thornLoadDetour = std::make_unique<FunctionDetour<ScriptLoadPtr>>(
        reinterpret_cast<ScriptLoadPtr>(GetProcAddress(common, "?ThornScriptLoad@@YAPAUIScriptEngine@@PBD@Z"))); // NOLINT

    thornLoadDetour->Detour(OnScriptLoadHook);

    if (const HMODULE d3d9 = GetModuleHandleA("d3d9.dll"))
    {
        d3d8CreateDetour = std::make_unique<FunctionDetour<Direct3DCreate9Ptr>>(reinterpret_cast<Direct3DCreate9Ptr>(GetProcAddress(d3d9, "Direct3DCreate9")));
        d3d8CreateDetour->Detour(OnDirect3D8Create);
    }

    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    frameUpdateDetour = std::make_unique<FunctionDetour<FrameUpdatePtr>>(reinterpret_cast<FrameUpdatePtr>(fl + 0x1B2890));
    frameUpdateDetour->Detour(OnUpdate);
}

FlufUi::~FlufUi() { hudManager.reset(); }
