#include "PCH.hpp"

#include <FLCore/FLCoreDefs.hpp>

#include "FLUF.UI.hpp"
#include "Rml/Interfaces/FileInterface.hpp"
#include "Rml/Interfaces/RenderInterfaceDirectX9.hpp"
#include "Rml/Interfaces/RmlInterface.hpp"

#include "FLUF/Include/Fluf.hpp"
#include "Rml/Interfaces/SystemInterface.hpp"
#include <FLCore/Common/INI_Reader.hpp>
#include <Utils/Detour.hpp>
#include <Utils/Utils.hpp>

#include "RmlUi/Core/Core.h"

#include "Rml/RmlWin32.hpp"
#include "RmlUi/Debugger/Debugger.h"
#include "RmlUi/Lua.h"

RmlInterface* module = nullptr;

bool RmlInterface::WndProc(const HWND hWnd, const uint msg, const WPARAM wParam, const LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYDOWN:
            {
                const Rml::Input::KeyIdentifier rml_key = RmlWin32::ConvertKey(static_cast<int>(wParam));
                const int rml_modifier = RmlWin32::GetKeyModifierState();
                const auto dpi = GetDpiForWindow(hWnd);
                const float native_dp_ratio = static_cast<float>(dpi == 0 ? USER_DEFAULT_SCREEN_DPI : dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);

                // See if we have any global shortcuts that take priority over the context.
                if (!RmlWin32::ProcessKeyDownShortcuts(ui, rmlContext, rml_key, rml_modifier, native_dp_ratio, true))
                {
                    return false;
                }

                rmlContext->ProcessKeyDown(RmlWin32::ConvertKey(static_cast<int>(wParam)), RmlWin32::GetKeyModifierState());

                // The key was not consumed by the context either, try keyboard shortcuts of lower priority.
                if (!RmlWin32::ProcessKeyDownShortcuts(ui, rmlContext, rml_key, rml_modifier, native_dp_ratio, false))
                {
                    return false;
                }

                break;
            }

        case WM_PAINT:
        case WM_QUIT:
        case WM_DESTROY:
        case WM_SIZE:
        case WM_SIZING:
        case WM_SHOWWINDOW:
        case WM_SETFOCUS:
            {
                Rml::ReleaseCompiledGeometry();
                Rml::ReleaseTextures();

                if (msg == WM_DESTROY || msg == WM_QUIT)
                {
                    shutDown = true;
                    Rml::Debugger::Shutdown();
                    Rml::Shutdown();
                }
                break;
            }
        case WM_KEYUP:
            {
                rmlContext->ProcessKeyUp(RmlWin32::ConvertKey(static_cast<int>(wParam)), RmlWin32::GetKeyModifierState());
                break;
            }
        case WM_CHAR:
            {
                static wchar_t first_u16_code_unit = 0;

                const auto c = static_cast<wchar_t>(wParam);
                auto character = static_cast<Rml::Character>(c);

                // Windows sends two-wide characters as two messages.
                if (c >= 0xD800 && c < 0xDC00)
                {
                    // First 16-bit code unit of a two-wide character.
                    first_u16_code_unit = c;
                }
                else
                {
                    if (c >= 0xDC00 && c < 0xE000 && first_u16_code_unit != 0)
                    {
                        // Second 16-bit code unit of a two-wide character.
                        const Rml::String utf8 = StringUtils::wstos(std::wstring{ first_u16_code_unit, c });
                        character = Rml::StringUtilities::ToCharacter(utf8.data(), utf8.data() + utf8.size());
                    }
                    else if (c == '\r')
                    {
                        // Windows sends new-lines as carriage returns, convert to endlines.
                        character = static_cast<Rml::Character>('\n');
                    }

                    first_u16_code_unit = 0;

                    // Only send through printable characters.
                    if ((static_cast<char32_t>(character) >= 32 || character == static_cast<Rml::Character>('\n')) &&
                        character != static_cast<Rml::Character>(127))
                    {
                        rmlContext->ProcessTextInput(character);
                    }
                }
                break;
            }
    }

    return true;
}

bool RmlInterface::WinKey(const uint msg, const WPARAM wParam, const LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYUP:
        case WM_KEYDOWN:
        case WM_CHAR:
            {
                const auto el = rmlContext->GetFocusElement();
                if (!el || el->GetTagName() != "input")
                {
                    break;
                }

                const auto attr = el->GetAttribute("type");
                if (!attr || attr->GetType() != Rml::Variant::STRING)
                {
                    break;
                }

                if (const auto attrValue = attr->Get<Rml::String>(); attrValue == "text" || attrValue == "password")
                {
                    return false;
                }
            }
        default: break;
    }

    return true;
}

void RmlInterface::PollMouse()
{
    static auto mouseX = reinterpret_cast<PINT>(0x616840);
    static auto mouseY = reinterpret_cast<PINT>(0x616844);
    static auto mouseStateRaw = reinterpret_cast<PDWORD>(0x616850);
    int* mouseZ = reinterpret_cast<int*>(0x616848);

    const auto mouseState = *mouseStateRaw;
    const auto xPos = *mouseX;
    const auto yPos = *mouseY;
    const int scroll = *mouseZ;

    currentMouseState = { (mouseState & 1) != 0, (mouseState & 2) != 0, (mouseState & 4) != 0, (mouseState & 8) != 0, (mouseState & 16) != 0 };

    // TODO: Handle keyboard modifier
    rmlContext->ProcessMouseMove(xPos, yPos, RmlWin32::GetKeyModifierState());

    for (int i = 0; i < currentMouseState.size(); ++i)
    {
        const bool down = currentMouseState[i];
        if (down != lastMouseState[i])
        {
            down ? rmlContext->ProcessMouseButtonDown(i, 0) : rmlContext->ProcessMouseButtonUp(i, RmlWin32::GetKeyModifierState());
        }
    }

    if (scroll)
    {
        rmlContext->ProcessMouseWheel(Rml::Vector2f{ 0.f, static_cast<float>(scroll) }, RmlWin32::GetKeyModifierState());
        *mouseZ = 0;
    }

    lastMouseState = currentMouseState;
}

void RmlInterface::PollKeyboard() {}

void RmlInterface::PollInput()
{
    PollMouse();
    PollKeyboard();
}

void RmlInterface::LoadFonts()
{
    INI_Reader ini;
    if (!ini.open(std::string(R"(\..\DATA\FONTS\fonts.ini)").c_str(), false))
    {
        goto fallback;
    }

    while (ini.read_header())
    {
        if (!ini.is_header("FontFiles"))
        {
            continue;
        }

        while (ini.read_value())
        {
            if (!ini.is_value("file"))
            {
                continue;
            }

            std::string value = ini.get_value_string(0);
            std::ranges::replace(value, '/', '\\');
            std::string file = R"(\..\DATA\)" + value;
            if (!std::filesystem::exists(file))
            {
                continue;
            }

            // Try to add the font
            if (const int result = AddFontResourceExA(file.c_str(), FR_PRIVATE, nullptr); result != 0)
            {
                if (auto [font, inserted] = fonts.insert(file); inserted)
                {

                    if (Rml::LoadFontFace("file://" + file, ini.get_value_bool(1)))
                    {
                        Fluf::Log(LogLevel::Info, std::format("Font loaded: {}", *font));
                    }
                    else
                    {
                        Fluf::Log(LogLevel::Warn, std::format("Failed to load font: {}", *font));
                    }
                }
            }
        }
    }

fallback:
    std::array<std::string, 9> systemFonts = {
        "segoeui.ttf",  // Segoe UI (Latin; Greek; Cyrillic; Armenian; Georgian; Georgian Khutsuri; Arabic; Hebrew; Fraser)
        "tahoma.ttf",   // Tahoma (Latin; Greek; Cyrillic; Armenian; Hebrew; Arabic; Thai)
        "meiryo.ttc",   // Meiryo UI (Japanese)
        "msgothic.ttc", // MS Gothic (Japanese)
        "msjh.ttc",     // Microsoft JhengHei (Chinese Traditional; Han; Han with Bopomofo)
        "msyh.ttc",     // Microsoft YaHei (Chinese Simplified; Han)
        "malgun.ttf",   // Malgun Gothic (Korean)
        "simsun.ttc",   // SimSun (Han Simplified)
        "seguiemj.ttf", // Segoe UI (Latin; Greek; Cyrillic; Armenian; Georgian; Georgian Khutsuri; Arabic; Hebrew; Fraser)
    };

    for (auto& font : systemFonts)
    {
        std::array<char, MAX_PATH> windowsDir;
        const auto len = GetSystemDirectoryA(windowsDir.data(), windowsDir.size());
        auto f = fonts.insert(std::format(R"(file://{}\..\Fonts\{})", std::string_view(windowsDir.data(), len), font));
        if (f.second)
        {
            if (Rml::LoadFontFace(*f.first, true))
            {
                Fluf::Log(LogLevel::Info, std::format("Font loaded: {}", *f.first));
            }
            else
            {
                Fluf::Log(LogLevel::Warn, std::format("Failed to load font: {}", *f.first));
            }
        }
    }
}

RmlInterface::RmlInterface(FlufUi* fluf, IDirect3D9* d3d9, IDirect3DDevice9* device)
    : ui(fluf), renderInterface(std::make_unique<RenderInterfaceDirectX9>(d3d9, device))
{
    module = this;

    // TODO: Support OpenGL as well
    systemInterface = std::make_unique<SystemInterface>();
    fileInterface = std::make_unique<FileInterface>();

    SetSystemInterface(systemInterface.get());
    SetFileInterface(fileInterface.get());
    SetRenderInterface(renderInterface.get());
    Rml::Initialise();
    Rml::Lua::Initialise();

    const auto freelancerHwnd = reinterpret_cast<HWND*>(0x67ECA0);
    RECT rect;
    GetWindowRect(*freelancerHwnd, &rect);

    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    rmlContext = Rml::CreateContext("main", Rml::Vector2i(width, height));
    LoadFonts();

    Rml::Debugger::Initialise(rmlContext);
#ifdef _DEBUG
    Rml::Debugger::SetVisible(true);
#endif

    rmlContext->SetDensityIndependentPixelRatio(ui->GetConfig(true)->dpi);
}

RmlContext RmlInterface::GetRmlContext() { return { rmlContext }; }
RmlInterface::~RmlInterface()
{
    if (!shutDown)
    {
        Rml::ReleaseCompiledGeometry();
        Rml::ReleaseTextures();
        Rml::Debugger::Shutdown();
        Rml::Shutdown();
    }
    module = nullptr;
}

void RmlInterface::Render()
{
    PollInput();
    rmlContext->Update();
    rmlContext->Render();
}
