#include "PCH.hpp"

#include "FLUF.UI.hpp"

#include "ClientServerCommunicator.hpp"
#include "Fluf.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "ImGui/ImGuiNotify.hpp"
#include "Typedefs.hpp"
#include "Internal/CustomOptionsWindow.hpp"
#include "Utils/Detour.hpp"
#include "Vanilla/HudManager.hpp"

#include <d3dx9.h>
#include <imgui_impl_dx9.h>

using ScriptLoadPtr = void* (*)(const char* fileName);
using FrameUpdatePtr = void (*)(double delta);
using Direct3DCreate9Ptr = IDirect3D9*(__stdcall*)(uint sdkVersion);
using Direct3DCreateDevice9 = HRESULT(__stdcall*)(IDirect3D9* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                  D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice9** returnedDeviceInterface);
using Direct3DDevice9Reset = HRESULT(__stdcall*)(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
using Direct3DDevice9EndScene = HRESULT(__stdcall*)(IDirect3DDevice9* device);

std::unique_ptr<FunctionDetour<Direct3DCreate9Ptr>> d3d8CreateDetour;
std::unique_ptr<FunctionDetour<Direct3DCreateDevice9>> d3d9CreateDeviceDetour;
std::unique_ptr<FunctionDetour<Direct3DDevice9Reset>> d3d9DeviceResetDetour;

FlufUi* module;

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
        renderingBackend = RenderingBackend::Dx9;
        if (config->uiMode == UiMode::ImGui)
        {
            Fluf::Log(LogLevel::Info, "Create ImGuiInterface w/ DX9");
            imguiInterface = std::make_shared<ImGuiInterface>(this, RenderingBackend::Dx9, d3d9device);
        }
    }
    else if (auto glContext = wglGetCurrentContext())
    {
        renderingBackend = RenderingBackend::OpenGL;
        if (config->uiMode == UiMode::ImGui)
        {
            Fluf::Log(LogLevel::Info, "Create ImGuiInterface w/ OpenGL");
            imguiInterface = std::make_shared<ImGuiInterface>(this, renderingBackend, glContext);
        }
    }
    else if (config->uiMode == UiMode::ImGui)
    {
        config->uiMode = UiMode::None;
        MessageBoxA(nullptr, "DirectX 9 not loaded. D3D8to9 must be present. Please disable ImGui interfaces, or use D3D8to9", "D3D8to9 not found", MB_OK);
        if (config->enforceUiMode)
        {
            std::exit(0);
        }
    }

    Fluf::Log(LogLevel::Info, std::format("UI Mode: {}", rfl::enum_to_string(config->uiMode)));
    Fluf::Log(LogLevel::Info, std::format("Rendering Backend: {}", rfl::enum_to_string(renderingBackend)));

    // Assuming success, create our submenus
    if (imguiInterface)
    {
        imguiInterface->InitSubmenus();
    }

    //Send a mouse click to initialize the window sizes on ImGui side of things.
    ImGuiInterface::WndProc(module->config.get(), *mainFreelancerWindow, WM_MOUSEMOVE, 0, 0x00010001);
}

LRESULT __stdcall FlufUi::WndProc(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam)
{
    if (module->imguiInterface)
    {
        if (const auto result = ImGuiInterface::WndProc(module->config.get(), hWnd, msg, wParam, lParam); !result)
        {
            return 0;
        }
    }

    module->wndProcDetour.UnDetour();
    const auto result = module->wndProcDetour.GetOriginalFunc()(hWnd, msg, wParam, lParam);
    module->wndProcDetour.Detour(WndProc);

    return result;
}

bool FlufUi::WinKeyDetour(const uint msg, const WPARAM wParam, const LPARAM lParam)
{
    module->winKeyDetour.UnDetour();
    const auto result = module->winKeyDetour.GetOriginalFunc()(msg, wParam, lParam);
    module->winKeyDetour.Detour(WinKeyDetour);

    return result;
}

bool FlufUi::UiRenderDetour()
{
    if (module->imguiInterface)
    {
        module->imguiInterface->Render();
    }

    module->uiRenderDetour.UnDetour();
    const auto result = module->uiRenderDetour.GetOriginalFunc()();
    module->uiRenderDetour.Detour(UiRenderDetour);
    return result;
}

IDirect3D9* __stdcall FlufUi::OnDirect3D8Create(const uint sdkVersion)
{
    d3d8CreateDetour->UnDetour();
    d3d9 = d3d8CreateDetour->GetOriginalFunc()(sdkVersion);

    const auto vtable = reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(d3d9));
    d3d9CreateDeviceDetour = std::make_unique<FunctionDetour<Direct3DCreateDevice9>>(reinterpret_cast<Direct3DCreateDevice9>(vtable[16]));
    d3d9CreateDeviceDetour->Detour(OnDirect3D9CreateDevice);

    return d3d9;
}

HRESULT __stdcall FlufUi::OnDirect3D9ResetDevice(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();

    d3d9DeviceResetDetour->UnDetour();
    const auto res = d3d9DeviceResetDetour->GetOriginalFunc()(device, pPresentationParameters);
    d3d9DeviceResetDetour->Detour(OnDirect3D9ResetDevice);

    return res;
}

HRESULT __stdcall FlufUi::OnDirect3D9CreateDevice(IDirect3D9* context, const uint adapter, const D3DDEVTYPE deviceType, const HWND focusWindow,
                                                  const DWORD behaviorFlags, D3DPRESENT_PARAMETERS* presentationParameters,
                                                  IDirect3DDevice9** returnedDeviceInterface)
{
    d3d9CreateDeviceDetour->UnDetour();
    const auto result =
        d3d9CreateDeviceDetour->GetOriginalFunc()(context, adapter, deviceType, focusWindow, behaviorFlags, presentationParameters, returnedDeviceInterface);
    d3d9device = *returnedDeviceInterface;
    assert(d3d9device);

    const auto vtable = reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(d3d9device));
    d3d9DeviceResetDetour = std::make_unique<FunctionDetour<Direct3DDevice9Reset>>(reinterpret_cast<Direct3DDevice9Reset>(vtable[16]));
    d3d9DeviceResetDetour->Detour(OnDirect3D9ResetDevice);

    d3d9CreateDeviceDetour->Detour(OnDirect3D9CreateDevice);
    return result;
}

bool FlufUi::OpenOptionsMenu() const
{
    imguiInterface->customOptionsWindow->ToggleOpenState();
    return true;
}

FlufModule::ModuleProcessCode FlufUi::OnPayloadReceived(uint sourceClientId, const FlufPayload& payload)
{
    if (payload.header != "toast")
    {
        return ModuleProcessCode::ContinueUnhandled;
    }

    struct ToastPayload
    {
            std::string title;
            std::string content;
            ImGuiToastType toastType;
            int timeUntilDismiss;
            bool addSeparator;
    };

    auto convertedPayload = payload.Convert<ToastPayload>();
    if (!convertedPayload)
    {
        return ModuleProcessCode::Handled;
    }

    auto& [title, content, toastType, timeUntilDismiss, addSeparator] = convertedPayload.value();

    if (timeUntilDismiss <= 0)
    {
        timeUntilDismiss = INT_MAX;
    }

    if (content.size() > NOTIFY_MAX_MSG_LENGTH)
    {
        content.resize(NOTIFY_MAX_MSG_LENGTH);
    }

    if (title.size() > NOTIFY_MAX_MSG_LENGTH)
    {
        title.resize(NOTIFY_MAX_MSG_LENGTH);
    }

    ImGuiToast toast{ toastType, timeUntilDismiss };
    toast.setTitle(title.c_str());
    toast.setContent(content.c_str());
    toast.setSeperator(addSeparator);

    ImGui::InsertNotification(toast);
    return ModuleProcessCode::Handled;
}

std::weak_ptr<HudManager> FlufUi::GetHudManager() { return hudManager; }

std::shared_ptr<FlufUiConfig> FlufUi::GetConfig() { return config; }

ImGuiInterface* FlufUi::GetImGuiInterface() const { return imguiInterface.get(); }
RenderingBackend FlufUi::GetRenderingBackend() const { return renderingBackend; }

FlufUi::FlufUi()
{
    module = this;

    hudManager = std::make_shared<HudManager>();

    auto conf = ConfigHelper<FlufUiConfig, FlufUiConfig::configPath>::Load();
    assert(conf.has_value());
    config = std::make_shared<FlufUiConfig>(*conf);

    conf = ConfigHelper<FlufUiConfig, FlufUiConfig::configPathOverrides>::Load(true, false);
    if (conf.has_value())
    {
        config = std::make_shared<FlufUiConfig>(*conf);
    }

    if (const HMODULE d3d9Handle = GetModuleHandleA("d3d9.dll"))
    {
        d3d8CreateDetour =
            std::make_unique<FunctionDetour<Direct3DCreate9Ptr>>(reinterpret_cast<Direct3DCreate9Ptr>(GetProcAddress(d3d9Handle, "Direct3DCreate9")));
        d3d8CreateDetour->Detour(OnDirect3D8Create);
    }

    uiRenderDetour.Detour(UiRenderDetour);
    winKeyDetour.Detour(WinKeyDetour);
    wndProcDetour.Detour(WndProc);
}

FlufUi::~FlufUi() { hudManager.reset(); }

std::string_view FlufUi::GetModuleName() { return moduleName; }

SETUP_MODULE(FlufUi);
