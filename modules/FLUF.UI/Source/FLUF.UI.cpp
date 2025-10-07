#include "PCH.hpp"

#include "FLUF.UI.hpp"

#include "ClientServerCommunicator.hpp"
#include "Fluf.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "ImGui/ImGuiNotify.hpp"
#include "Typedefs.hpp"
#include "Internal/CustomOptionsWindow.hpp"
#include "Internal/ImGuiD3D8.hpp"
#include "Utils/Detour.hpp"
#include "Vanilla/HudManager.hpp"
#include "Exceptions.hpp"

#include <KeyManager.hpp>
#include <vendor/DXSDK/include/d3d8.h>

#undef interface

using Direct3DCreate8Ptr = IDirect3D8*(__stdcall*)(uint sdkVersion);
using Direct3DCreateDevice8 = HRESULT(__stdcall*)(IDirect3D8* context, uint adapter, D3DDEVTYPE deviceType, HWND focusWindow, DWORD behaviorFlags,
                                                  D3DPRESENT_PARAMETERS* presentationParameters, IDirect3DDevice8** returnedDeviceInterface);
using Direct3DDevice8Reset = HRESULT(__stdcall*)(IDirect3DDevice8* device, D3DPRESENT_PARAMETERS* pPresentationParameters);

std::unique_ptr<FunctionDetour<Direct3DCreate8Ptr>> d3d8CreateDetour;
std::unique_ptr<FunctionDetour<Direct3DCreateDevice8>> d3d8CreateDeviceDetour;
std::unique_ptr<FunctionDetour<Direct3DDevice8Reset>> d3d8DeviceResetDetour;

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
    if (d3d8 && d3d8device)
    {
        if (config->uiMode == UiMode::ImGui)
        {
            Fluf::Log(LogLevel::Info, "Create ImGuiInterface w/ DX8");
            imguiInterface = std::make_shared<ImGuiInterface>(this, RenderingBackend::Dx8, d3d8device);
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
    else
    {
        config->uiMode = UiMode::None;
    }

    Fluf::Log(LogLevel::Info, std::format("UI Mode: {}", rfl::enum_to_string(config->uiMode)));
    Fluf::Log(LogLevel::Info, std::format("Rendering Backend: {}", rfl::enum_to_string(renderingBackend)));

    // Assuming success, create our submenus
    if (imguiInterface)
    {
        imguiInterface->InitSubmenus();
        //Send a mouse click to initialize the window sizes on ImGui side of things.
        ImGuiInterface::WndProc(module->config.get(), *mainFreelancerWindow, WM_MOUSEMOVE, 0, 0x00010001);
    }
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

IDirect3D8* __stdcall FlufUi::OnDirect3D8Create(const uint sdkVersion)
{
    d3d8CreateDetour->UnDetour();
    d3d8 = d3d8CreateDetour->GetOriginalFunc()(sdkVersion);

    const auto vtable = reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(d3d8));
    d3d8CreateDeviceDetour = std::make_unique<FunctionDetour<Direct3DCreateDevice8>>(reinterpret_cast<Direct3DCreateDevice8>(vtable[15]));
    d3d8CreateDeviceDetour->Detour(OnDirect3D8CreateDevice);

    return d3d8;
}

HRESULT __stdcall FlufUi::OnDirect3D8ResetDevice(IDirect3DDevice8* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    module->imguiInterface->UnloadTextures();

    d3d8DeviceResetDetour->UnDetour();
    const auto res = d3d8DeviceResetDetour->GetOriginalFunc()(device, pPresentationParameters);
    d3d8DeviceResetDetour->Detour(OnDirect3D8ResetDevice);

    module->imguiInterface->ResetTextures();

    return res;
}

HRESULT __stdcall FlufUi::OnDirect3D8CreateDevice(IDirect3D8* context, const uint adapter, const D3DDEVTYPE deviceType, const HWND focusWindow,
                                                  const DWORD behaviorFlags, D3DPRESENT_PARAMETERS* presentationParameters,
                                                  IDirect3DDevice8** returnedDeviceInterface)
{
    d3d8CreateDeviceDetour->UnDetour();
    const auto result =
        d3d8CreateDeviceDetour->GetOriginalFunc()(context, adapter, deviceType, focusWindow, behaviorFlags, presentationParameters, returnedDeviceInterface);
    d3d8device = *returnedDeviceInterface;
    assert(d3d8device);

    const auto vtable = reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(d3d8device));
    d3d8DeviceResetDetour = std::make_unique<FunctionDetour<Direct3DDevice8Reset>>(reinterpret_cast<Direct3DDevice8Reset>(vtable[14]));
    d3d8DeviceResetDetour->Detour(OnDirect3D8ResetDevice);

    d3d8CreateDeviceDetour->Detour(OnDirect3D8CreateDevice);
    return result;
}

bool FlufUi::OpenOptionsMenu(KeyState state) const
{
    imguiInterface->customOptionsWindow->SetOpenState(!imguiInterface->customOptionsWindow->IsOpen());
    return true;
}

bool FlufUi::ProcessEscapeKey(KeyState state) const
{
    for (auto windowIter = imguiInterface->flWindowStack.rbegin(); windowIter != imguiInterface->flWindowStack.rend(); windowIter++)
    {
        auto window = *windowIter;
        if (window->IsEscapeCloseable())
        {
            window->SetOpenState(false);
            return true;
        }
    }

    return false;
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
    if (!Fluf::IsRunningOnClient())
    {
        throw ModuleLoadException("FLUF UI does not run in server context.");
    }

    module = this;

    hudManager = std::make_shared<HudManager>();

    auto conf = ConfigHelper<FlufUiConfig>::Load(FlufUiConfig::configPath);
    assert(conf.has_value());
    config = std::make_shared<FlufUiConfig>(*conf);

    conf = ConfigHelper<FlufUiConfig>::Load(FlufUiConfig::configPathOverrides, true, false);
    if (conf.has_value())
    {
        config = std::make_shared<FlufUiConfig>(*conf);
    }

    if (const HMODULE d3d8Handle = GetModuleHandleA("d3d8.dll"))
    {
        d3d8CreateDetour =
            std::make_unique<FunctionDetour<Direct3DCreate8Ptr>>(reinterpret_cast<Direct3DCreate8Ptr>(GetProcAddress(d3d8Handle, "Direct3DCreate8")));
        d3d8CreateDetour->Detour(OnDirect3D8Create);
    }

    uiRenderDetour.Detour(UiRenderDetour);
    winKeyDetour.Detour(WinKeyDetour);
    wndProcDetour.Detour(WndProc);
}

FlufUi::~FlufUi() { hudManager.reset(); }

std::string_view FlufUi::GetModuleName() { return moduleName; }

SETUP_MODULE(FlufUi);
