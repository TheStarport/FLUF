#include "PCH.hpp"

#include "ImGui/ImGuiInterface.hpp"

#include "FLUF.UI.hpp"
#include "FLUF/Include/Fluf.hpp"
#include "ImGui/ImGuiModule.hpp"

#include <d3dx9.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

void ImGuiInterface::Render(const std::unordered_set<ImGuiModule*>& imguiModules) const
{
    PollInput();

    switch (backend)
    {
        case RenderingBackend::Dx9: ImGui_ImplDX9_NewFrame(); break;
        default: throw std::runtime_error("Unknown backend");
    }

    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    for (auto* module : imguiModules)
    {
        module->Render();
    }

    ImGui::EndFrame();
    ImGui::Render();

    switch (backend)
    {
        case RenderingBackend::Dx9: ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData()); break;
        default: throw std::runtime_error("Unknown backend");
    }
}

ImGuiInterface::MouseState ImGuiInterface::ConvertState(const DWORD state)
{
    // clang-format off
    return {
        static_cast<bool>(state & 1),
        static_cast<bool>(state & 2),
        static_cast<bool>(state & 4),
        static_cast<bool>(state & 8),
        static_cast<bool>(state & 16),
    };
    // clang-format on
}

void ImGuiInterface::PollInput()
{
    static auto* mouseX = reinterpret_cast<PDWORD>(0x616840);
    static auto* mouseY = reinterpret_cast<PDWORD>(0x616844);
    static auto* mouseStateRaw = reinterpret_cast<PDWORD>(0x616850);
    static auto* mouseZ = reinterpret_cast<int*>(0x616848);

    constexpr int left = 0, right = 1, middle = 2, x1 = 3, x2 = 4;

    ImGuiIO& io = ImGui::GetIO();

    const DWORD xPos = *mouseX;
    const DWORD yPos = *mouseY;
    const int scroll = *mouseZ;

    const auto [leftDown, rightDown, middleDown, mouse4Down, mouse5Down] = ConvertState(*mouseStateRaw);

    // Position before anything else
    io.AddMousePosEvent(static_cast<float>(xPos), static_cast<float>(yPos));
    io.AddMouseButtonEvent(left, leftDown);
    io.AddMouseButtonEvent(right, rightDown);
    io.AddMouseButtonEvent(middle, middleDown);
    io.AddMouseButtonEvent(x1, mouse4Down);
    io.AddMouseButtonEvent(x2, mouse5Down);

    if (scroll)
    {
        io.AddMouseWheelEvent(0.f, static_cast<float>(scroll));
        *mouseZ = 0;
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool ImGuiInterface::WndProc(FlufUiConfig* config, const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
    if (msg == WM_KEYDOWN)
    {
        const char key = static_cast<char>(wParam);
        if (const bool hasCtl = GetKeyState(VK_CONTROL) & 1; hasCtl && key == VK_OEM_PLUS || key == VK_OEM_MINUS)
        {
            config->dpi = key == VK_OEM_PLUS ? std::max(config->dpi * 1.1f, 2.5f) : std::min(config->dpi / 1.1f, 0.5f);
            ImGui::GetCurrentContext()->CurrentDpiScale = config->dpi;
            ConfigHelper<FlufUiConfig, FlufUiConfig::configPathOverrides>::Save(*config);
            return false;
        }
    }

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return false;
    }

    return true;
}

ImGuiInterface::~ImGuiInterface()
{
    switch (backend)
    {
        case RenderingBackend::Dx9:
            // By the time this is unloaded, device is lost. Clear the flag to prevent an attempt from releasing a nonexistant resource.
            *static_cast<LPDIRECT3DDEVICE9*>(ImGui::GetIO().BackendRendererUserData) = nullptr;
            ImGui_ImplDX9_Shutdown();
            break;
        default: break;
    }

    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

ImGuiInterface::ImGuiInterface(FlufUi* flufUi, const RenderingBackend backend, void* device) : config(flufUi->GetConfig(true)), backend(backend)
{
    static const auto* mainFreelancerWindow = reinterpret_cast<HWND*>(0x6679F4);
    const auto ctx = ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ctx->CurrentDpiScale = config->dpi;

    // Disable cycling windows with keys
    ctx->ConfigNavWindowingKeyNext = 0;
    ctx->ConfigNavWindowingKeyPrev = 0;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NoMouseCursorChange;

    for (auto& loadedFont : config->loadedFonts)
    {
        for (auto fontSize : loadedFont.fontSizes)
        {
            auto* font = io.Fonts->AddFontFromFileTTF(std::format(R"(..\DATA\FONTS\{})", loadedFont.fontPath).c_str(), static_cast<float>(fontSize));
            assert(font);

            if (loadedFont.isDefault && fontSize == 14)
            {
                io.FontDefault = font;
            }

            loadedFont.fontSizesInternal.value()[fontSize] = font;
        }
    }

    Fluf::Log(LogLevel::Debug, std::format("Creating ImGui interface with backend: {}", rfl::enum_to_string(backend)));
    ImGui_ImplWin32_Init(*mainFreelancerWindow);
    switch (backend)
    {
        case RenderingBackend::Dx9:
            {
                const auto dx9Device = static_cast<IDirect3DDevice9*>(device);
                ImGui_ImplDX9_Init(dx9Device);
                break;
            }
        case RenderingBackend::Dx8:
        default: throw std::runtime_error("Dx8 backend not supported.");
    }

    if (config->loadedFonts.empty())
    {
        MessageBoxA(nullptr, "No fonts have been loaded into FLUF UI. Modules using ImGui may cause crashes.", "No Fonts Loaded", MB_OK);
    }

    // Load fonts with a default size of 14
    for (const auto& loadedFont : config->loadedFonts)
    {
        FlufUi::GetImGuiFont(loadedFont.fontName, 14); // Call get on size 14 to setup any needed defaults
    }
}
