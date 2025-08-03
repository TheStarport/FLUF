// ReSharper disable CppRedundantElseKeyword
#include "PCH.hpp"

#include "ImGui/ImGuiInterface.hpp"

#include "FLUF.UI.hpp"
#include "Fluf.hpp"
#include "ImGui/ImGuiModule.hpp"

#include "ImGui/FontAwesomeSolid.hpp"
#include "ImGui/ImGuiNotify.hpp"
#include <d3dx9.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>
#include <gl/GL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "KeyManager.hpp"
#include "UImGuiTextUtils.hpp"
#include "Internal/CustomHud.hpp"
#include "Internal/PlayerStatusWindow.hpp"
#include "Vanilla/HudManager.hpp"

#include <magic_enum.hpp>
#include <ImGui/IconFontAwesome6.hpp>
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

ImGuiStyle& ImGuiInterface::GenerateDefaultStyle()
{
    constexpr auto bgColor = ImVec4(0.0784f, 0.0431f, 0.2235f, 0.7f);      // #140B39
    constexpr auto bgColorChild = ImVec4(0.0784f, 0.0431f, 0.2235f, 0.0f); // #140B39

    constexpr auto titleBgColor = ImVec4(0.00784314, 0.105882, 0.188235, 1); // #021B30
    constexpr auto borderColor = ImVec4(0.50f, 0.72f, 0.83f, 1.0f);
    constexpr auto textColor = ImVec4(0.529f, 0.764f, 0.878f, 1.0f);
    constexpr auto checkBoxCheckColor = ImVec4(0.16f, 0.59f, 0.16f, 1.0f);        // #28962B
    constexpr auto scrollBarColor = ImVec4(0.411f, 0.596f, 0.678f, 1.0f);         // #6998AE
    constexpr auto disabledColor = ImVec4(0.451f, 0.451f, 0.463f, 1.0f);          // #737276
    constexpr auto borderShadowColor = ImVec4(0.4549f, 0.6235f, 0.7569f, 1.0f);   // #749FC1
    constexpr auto frameBg = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);                      // Black
    constexpr auto buttonNormalColor = ImVec4(0.00784314, 0.105882, 0.188235, 1); // #021B30
    constexpr auto buttonHoverColor = ImVec4(0.52f, 0.45f, 0.0f, 1.0f);           // Hex #857400
    constexpr auto comboHoverColor = ImVec4(0.52f, 0.45f, 0.0f, 1.0f);            // Hex #857400

    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.WindowPadding = ImVec2(8.f, 4.0f);
    style.WindowMinSize = ImVec2(20.0f, 20.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.SeparatorTextAlign = ImVec2(.5f, .5);
    style.SeparatorTextPadding = ImVec2(20.f, 3.f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(20.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
    style.IndentSpacing = 20.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 20.0f;
    style.GrabMinSize = 16.0f;
    style.TabBorderSize = 1.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style.WindowBorderSize = 2.0f;
    style.FramePadding = ImVec2(6.f, 2.0f);
    style.CellPadding = ImVec2(3.f, 2.0f);

    style.WindowRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 6.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 5.0f;

    style.Colors[ImGuiCol_Text] = textColor;
    style.Colors[ImGuiCol_TextDisabled] = disabledColor;
    style.Colors[ImGuiCol_WindowBg] = bgColor;
    style.Colors[ImGuiCol_ChildBg] = bgColorChild;
    style.Colors[ImGuiCol_PopupBg] = bgColor;
    style.Colors[ImGuiCol_Border] = borderColor;
    style.Colors[ImGuiCol_BorderShadow] = borderShadowColor;
    style.Colors[ImGuiCol_FrameBg] = frameBg;
    style.Colors[ImGuiCol_FrameBgHovered] = frameBg;
    style.Colors[ImGuiCol_FrameBgActive] = frameBg;
    style.Colors[ImGuiCol_TitleBg] = titleBgColor;
    style.Colors[ImGuiCol_TitleBgActive] = titleBgColor;
    style.Colors[ImGuiCol_TitleBgCollapsed] = titleBgColor;

    style.Colors[ImGuiCol_MenuBarBg] = bgColor;
    style.Colors[ImGuiCol_ScrollbarBg] = bgColor;
    style.Colors[ImGuiCol_ScrollbarGrab] = scrollBarColor;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = comboHoverColor;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = comboHoverColor;
    style.Colors[ImGuiCol_CheckMark] = checkBoxCheckColor;
    style.Colors[ImGuiCol_SliderGrab] = scrollBarColor;
    style.Colors[ImGuiCol_SliderGrabActive] = comboHoverColor;
    style.Colors[ImGuiCol_Button] = buttonNormalColor;
    style.Colors[ImGuiCol_ButtonHovered] = buttonHoverColor;
    style.Colors[ImGuiCol_ButtonActive] = buttonHoverColor;
    style.Colors[ImGuiCol_Header] = bgColor;
    style.Colors[ImGuiCol_HeaderHovered] = comboHoverColor;
    style.Colors[ImGuiCol_HeaderActive] = comboHoverColor;
    style.Colors[ImGuiCol_Separator] = scrollBarColor;
    style.Colors[ImGuiCol_SeparatorHovered] = scrollBarColor;
    style.Colors[ImGuiCol_SeparatorActive] = scrollBarColor;
    style.Colors[ImGuiCol_ResizeGrip] = bgColor;
    style.Colors[ImGuiCol_ResizeGripHovered] = titleBgColor;
    style.Colors[ImGuiCol_ResizeGripActive] = bgColor;
    style.Colors[ImGuiCol_Tab] = titleBgColor;
    style.Colors[ImGuiCol_TabHovered] = buttonHoverColor;
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.f, 0.22f, 0.43f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = titleBgColor;
    style.Colors[ImGuiCol_TabUnfocusedActive] = titleBgColor;

    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = titleBgColor;
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0);

    return style;
}

void ImGuiInterface::InitSubmenus()
{
    // Init submenus
    playerStatusWindow = std::make_unique<PlayerStatusWindow>(statMenus);

    // Register custom hud for listening to the player status button
    customHud = std::make_unique<CustomHud>(playerStatusWindow.get());
    flufUi->GetHudManager().lock()->RegisterHud(customHud.get());

    // Dummy menus for needed categories (these menus are integrated directly into PlayerStatusMenu)
    RegisterStatsMenu(flufUi, "Exploration", nullptr);
    RegisterStatsMenu(flufUi, "Kill Counts", nullptr);
}

void ImGuiInterface::Render()
{
    PollInput();

    switch (backend)
    {
        case RenderingBackend::Dx9: ImGui_ImplDX9_NewFrame(); break;
        case RenderingBackend::OpenGL: ImGui_ImplOpenGL3_NewFrame(); break;
        default: throw std::runtime_error("Unknown backend");
    }

    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetShortcutRouting(ImGuiMod_Ctrl | ImGuiKey_Tab, ImGuiInputFlags_None, ImGuiInputFlags_RouteGlobal);
    ImGui::SetShortcutRouting(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Tab, ImGuiInputFlags_None, ImGuiInputFlags_RouteGlobal);
    ImGui::SetShortcutRouting(ImGuiKey_Tab, ImGuiInputFlags_None, ImGuiInputFlags_RouteGlobal);

    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    RenderOptionsMenu();

    for (auto* module : imguiModules)
    {
        module->Render();
    }

    playerStatusWindow->Render();

    // Render notifications above all else
    ImGui::RenderNotifications();

    ImGui::Render();

    switch (backend)
    {
        case RenderingBackend::Dx9: ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData()); break;
        case RenderingBackend::OpenGL: ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); break;
        default: throw std::runtime_error("Unknown backend");
    }
}

void ImGuiInterface::RenderOptionsMenu()
{
    using namespace std::string_view_literals;
    if (!showOptionsWindow)
    {
        return;
    }

    static std::string_view lastSavedModule;
    ImGui::SetNextWindowSize({ 1280.f, 1024.f }, ImGuiCond_Appearing);
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, 4.f);
    if (ImGui::Begin("Extended Options Menu", &showOptionsWindow))
    {
        if (ImGui::BeginTabBar("##option-tabs"))
        {
            const auto itemSpacing = ImGui::GetStyle().ItemSpacing;
            int counter = 0;
            for (const auto& menu : registeredOptionMenus)
            {
                const auto nameView = menu.first->GetModuleName();
                auto name = std::string(nameView) + "##";
                ImGui::PushID(counter++);
                if (ImGui::BeginTabItem(name.c_str()))
                {
                    constexpr auto saveChangesText = "Save Changes"sv;
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
                                         (ImGui::CalcTextSize(saveChangesText.data()).x + itemSpacing.x * 2.0f));
                    const auto saveRequested = ImGui::Button(saveChangesText.data());

                    (menu.first->*menu.second)(saveRequested);

                    if (saveRequested)
                    {
                        Fluf::Log(LogLevel::Info, "Saved changes");
                        lastSavedModule = nameView;
                        ImGui::OpenPopup("Changes Saved!");
                    }

                    if (ImGui::BeginPopupModal("Changes Saved!", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        std::string text = std::format("Successfully saved settings for {}", lastSavedModule);
                        ImGui::Text(text.c_str());
                        if (ImGui::Button("OK"))
                        {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::EndTabItem();
                }
                ImGui::PopID();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
    ImGui::PopStyleVar();
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
        else if (hasCtl && key == VK_F5)
        {
            showDemoWindow = true;
            return false;
        }
    }

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return false;
    }

    return true;
}

void* ImGuiInterface::GetRenderingContext() const { return renderingContext; }

ImGuiInterface::~ImGuiInterface()
{
    switch (backend)
    {
        case RenderingBackend::Dx9:
            // By the time this is unloaded, device is lost. Clear the flag to prevent an attempt from releasing a nonexistant resource.
            *static_cast<LPDIRECT3DDEVICE9*>(ImGui::GetIO().BackendRendererUserData) = nullptr;
            ImGui_ImplDX9_Shutdown();
            break;
        case RenderingBackend::OpenGL: ImGui_ImplOpenGL3_Shutdown(); break;
        default: break;
    }

    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

ImGuiInterface::ImGuiInterface(FlufUi* flufUi, const RenderingBackend backend, void* context)
    : renderingContext(context), flufUi(flufUi), config(flufUi->GetConfig()), backend(backend)
{
    std::array<char, MAX_PATH> path{};
    GetUserDataPath(path.data());

    iniPath = std::format("{}\\imgui.ini", path.data());

    static const auto* mainFreelancerWindow = reinterpret_cast<HWND*>(0x6679F4);
    const auto ctx = ImGui::CreateContext();

    GenerateDefaultStyle();

    ctx->CurrentDpiScale = config->dpi;

    // Disable cycling windows with keys
    ctx->ConfigNavWindowingKeyNext = 0;
    ctx->ConfigNavWindowingKeyPrev = 0;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NoMouseCursorChange;
    io.IniFilename = iniPath.c_str();

    for (auto& loadedFont : config->loadedFonts)
    {
        if (!loadedFont.fontSizes)
        {
            loadedFont.fontSizes = std::set<int>{};
        }

        // Ensure the default sizes are present
        for (auto size : magic_enum::enum_values<FontSize>())
        {
            loadedFont.fontSizes->insert(static_cast<int>(size));
        }

        auto addFa = [](const float fontSize)
        {
            static constexpr ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };

            ImFontConfig fontConfig;
            const float iconFontSize = fontSize * 0.666666666f;
            fontConfig.MergeMode = true;
            fontConfig.PixelSnapH = true;
            fontConfig.GlyphMinAdvanceX = iconFontSize;
            ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FontAwesomeCompressedData, FontAwesomeCompressedSize, iconFontSize, &fontConfig, iconRanges);
        };

        for (auto fontSize : *loadedFont.fontSizes)
        {
            std::string fontPath = std::format(R"(..\DATA\FONTS\{})", loadedFont.fontPath);
            if (!std::filesystem::exists(fontPath) && !std::filesystem::is_regular_file(fontPath))
            {
                Fluf::Warn(std::format("Unable to load font: {}", fontPath));
                continue;
            }

            auto fontSizeFloat = static_cast<float>(fontSize);
            auto* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSizeFloat);
            assert(font);

            if (loadedFont.isDefault && fontSize == static_cast<int>(FontSize::Default))
            {
                io.FontDefault = font;
            }

            addFa(fontSizeFloat);
            loadedFont.fontSizesInternal.value()[fontSize] = font;
        }

        if (loadedFont.isDefault)
        {
            static UImGui::TextUtilsData data;

            std::filesystem::path fontPath = std::format(R"(..\DATA\FONTS\{})", loadedFont.fontPath);
            auto lightPath = fontPath.parent_path().append(fontPath.stem().string()).string() + +"-Light" + fontPath.extension().string();
            auto boldPath = fontPath.parent_path().append(fontPath.stem().string()).string() + "-Bold" + fontPath.extension().string();
            auto italicPath = fontPath.parent_path().append(fontPath.stem().string()).string() + "-Italic" + fontPath.extension().string();
            auto boldAndItalicPath = fontPath.parent_path().append(fontPath.stem().string()).string() + "-BoldItalic" + fontPath.extension().string();

            ImFont* lightFont = io.FontDefault;
            ImFont* boldFont = io.FontDefault;
            ImFont* italicFont = io.FontDefault;
            ImFont* boldItalicFont = io.FontDefault;

            constexpr auto fontSize = static_cast<float>(FontSize::Default);

            if (std::filesystem::exists(lightPath))
            {
                lightFont = io.Fonts->AddFontFromFileTTF(lightPath.c_str(), fontSize);
                addFa(fontSize);
            }

            if (std::filesystem::exists(boldPath))
            {
                boldFont = io.Fonts->AddFontFromFileTTF(boldPath.c_str(), fontSize);
                addFa(fontSize);
            }

            if (std::filesystem::exists(italicPath))
            {
                italicFont = io.Fonts->AddFontFromFileTTF(italicPath.c_str(), fontSize);
                addFa(fontSize);
            }

            if (std::filesystem::exists(boldAndItalicPath))
            {
                boldItalicFont = io.Fonts->AddFontFromFileTTF(boldAndItalicPath.c_str(), fontSize);
                addFa(fontSize);
            }

            data = {
                .bold = boldFont,
                .italic = italicFont,
                .boldItalic = boldItalicFont,
                .smallFont = lightFont,
            };

            UImGui::TextUtils::initTextUtilsData(&data);
        }
    }

    Fluf::Log(LogLevel::Debug, std::format("Creating ImGui interface with backend: {}", rfl::enum_to_string(backend)));
    ImGui_ImplWin32_Init(*mainFreelancerWindow);
    switch (backend)
    {
        case RenderingBackend::Dx9:
            {
                const auto dx9Device = static_cast<IDirect3DDevice9*>(context);
                ImGui_ImplDX9_Init(dx9Device);
                break;
            }
        case RenderingBackend::OpenGL:
            {
                ImGui_ImplOpenGL3_Init();
                break;
            }
        case RenderingBackend::Dx8:
        default: throw std::runtime_error("Dx8 backend not supported.");
    }

    if (config->loadedFonts.empty())
    {
        MessageBoxA(nullptr, "No fonts have been loaded into FLUF UI. Modules using ImGui may cause crashes.", "No Fonts Loaded", MB_OK);
    }

    Fluf::GetKeyManager()->RegisterKey(flufUi, "FLUF_OPEN_EXTENDED_OPTIONS_MENU", Key::USER_NO_OVERRIDE, reinterpret_cast<KeyFunc>(&FlufUi::OpenOptionsMenu));
}

void* ImGuiInterface::LoadTexture(const std::string& path, uint& width, uint& height)
{
    if (const auto texture = loadedTextures.find(path); texture != loadedTextures.end())
    {
        return texture->second;
    }

    if (backend == RenderingBackend::Dx9)
    {
        PDIRECT3DTEXTURE9 d3dTexture = nullptr;
        if (const auto hr = D3DXCreateTextureFromFileA(static_cast<LPDIRECT3DDEVICE9>(renderingContext), path.c_str(), &d3dTexture); hr != D3D_OK)
        {
            goto failed;
        }

        D3DSURFACE_DESC surfaceDesc = {};
        if (const auto hr = d3dTexture->GetLevelDesc(0, &surfaceDesc); hr != D3D_OK)
        {
            d3dTexture->Release();
            goto failed;
        }

        loadedTextures[path] = d3dTexture;

        width = surfaceDesc.Width;
        height = surfaceDesc.Height;
        return d3dTexture;
    }

    if (backend == RenderingBackend::OpenGL)
    {
        FILE* f = fopen(path.c_str(), "rb");
        if (f == nullptr)
        {
            goto failed;
        }

        auto imageHandle = stbi_load_from_file(f, reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height), nullptr, 4);
        if (!imageHandle)
        {
            fclose(f);
            goto failed;
        }

        // Create a OpenGL texture identifier
        GLuint imageTexture;
        glGenTextures(1, &imageTexture);
        glBindTexture(GL_TEXTURE_2D, imageTexture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload pixels into texture
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageHandle);
        stbi_image_free(imageHandle);
        fclose(f);

        loadedTextures[path] = reinterpret_cast<void*>(imageTexture);
        return reinterpret_cast<void*>(imageTexture);
    }

failed:
    // Cache the fact we did not find it to prevent duplicate logs
    loadedTextures[path] = nullptr;

    Fluf::Log(LogLevel::Error, std::format("Failed to load texture: {}", path));

    return nullptr;
}

bool ImGuiInterface::RegisterImGuiModule(ImGuiModule* mod)
{
    Fluf::Log(LogLevel::Info, "Registering ImGui module");
    return imguiModules.insert(mod).second;
}

bool ImGuiInterface::UnregisterImGuiModule(ImGuiModule* mod)
{
    Fluf::Log(LogLevel::Info, "Unregistering ImGui module");
    return imguiModules.erase(mod) == 1;
}

ImFont* ImGuiInterface::GetImGuiFont(const std::string& fontName, const int fontSize) const
{
    auto& loadedImGuiFonts = config->loadedFonts;
    if (loadedImGuiFonts.empty())
    {
        throw std::runtime_error("FlufUi::GetImGuiFont: No fonts loaded");
    }

    const auto loadedFont = std::ranges::find_if(loadedImGuiFonts, [fontName](const LoadedFont& font) { return font.fontName == fontName; });
    if (loadedFont == loadedImGuiFonts.end())
    {
        MessageBoxA(nullptr, std::format("Font {} not found or failed to load.", fontName).c_str(), "Font Error", MB_OK);
        return nullptr;
    }

    auto& fontSizes = loadedFont->fontSizesInternal.value();
    const auto size = fontSizes.find(fontSize);
    if (size == fontSizes.end())
    {
        MessageBoxA(nullptr, std::format("Font {} of size {} not found or failed to load.", fontName, fontSize).c_str(), "Font Error", MB_OK);
        return nullptr;
    }

    return size->second;
}

ImFont* ImGuiInterface::GetImGuiFont(const std::string& fontName, const FontSize fontSize) const { return GetImGuiFont(fontName, static_cast<int>(fontSize)); }

ImFont* ImGuiInterface::GetDefaultFont(FontSize fontSize) const { return GetDefaultFont(static_cast<int>(fontSize)); }

ImFont* ImGuiInterface::GetDefaultFont(int fontSize) const
{
    if (fontSize <= 0)
    {
        fontSize = static_cast<int>(FontSize::Default);
    }

    for (const auto& font : config->loadedFonts)
    {
        if (!font.isDefault)
        {
            continue;
        }

        if (!font.fontSizes)
        {
            throw std::runtime_error("Trying to access default fonts before initialisation is invalid");
        }

        if (!font.fontSizes->contains(fontSize))
        {
            throw std::runtime_error("Default font doesn't contain specified font size");
        }

        return font.fontSizesInternal.value().find(fontSize)->second;
    }

    throw std::runtime_error("No font was specified as default");
}

bool ImGuiInterface::RegisterOptionsMenu(FlufModule* module, const RegisterOptionsFunc function)
{
    if (registeredOptionMenus.contains(module))
    {
        return false;
    }

    Fluf::Log(LogLevel::Info, std::format("({}) Registering Option Menu", module->GetModuleName()));
    registeredOptionMenus[module] = function;
    return true;
}

bool ImGuiInterface::RegisterStatusMenu(FlufModule* module, RegisterMenuFunc function) const
{
    if (registeredOptionMenus.contains(module))
    {
        return false;
    }

    playerStatusWindow->RegisterNewMenu(module, function);
    return true;
}

bool ImGuiInterface::RegisterStatsMenu(FlufModule* module, const std::string& category, OnRenderStatsMenu function)
{
    const auto iter = statMenus.find(category);
    if (iter == statMenus.end())
    {
        Fluf::Info(std::format("Registering new stats category: {}.", category));
        statMenus[category] = {
            { module, { function } }
        };

        return true;
    }

    if (const auto subList = iter->second.find(module); subList != iter->second.end())
    {
        Fluf::Error("Tried to register stats menu category multiple times.");
        return false;
    }

    iter->second[module] = function;
    return true;
}
