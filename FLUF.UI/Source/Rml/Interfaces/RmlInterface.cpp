#include "PCH.hpp"

#include <FLCore/FLCoreDefs.hpp>

#include "Rml/Interfaces/FileInterface.hpp"
#include "Rml/Interfaces/RenderInterfaceDirectX9.hpp"
#include "Rml/Interfaces/RmlInterface.hpp"

#include "Rml/Interfaces/SystemInterface.hpp"
#include <FLCore/Common/INI_Reader.hpp>
#include <Utils/Detour.hpp>
#include <Utils/Utils.hpp>

#include "RmlUi/Core/Core.h"
#include "RmlUi/Debugger/Debugger.h"
// #include "RmlUi/Lua.h"

#include "Rml/RmlWin32.hpp"

RmlInterface* module = nullptr;

using WinKeyType = bool (*)(uint msg, WPARAM wParam, LPARAM lParam);
using OriginalWndProc = LRESULT(__stdcall*)(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);
FunctionDetour winKeyDetour(reinterpret_cast<WinKeyType>(0x577850));
FunctionDetour wndProcDetour(reinterpret_cast<OriginalWndProc>(0x5B2570));

LRESULT __stdcall RmlInterface::WndProc(const HWND hWnd, const uint msg, const WPARAM wParam, const LPARAM lParam)
{
    if (msg == WM_KEYDOWN)
    {
        const Rml::Input::KeyIdentifier rml_key = RmlWin32::ConvertKey((int)wParam);
        const int rml_modifier = RmlWin32::GetKeyModifierState();
        auto dpi = GetDpiForWindow(hWnd);
        const float native_dp_ratio = float(dpi == 0 ? USER_DEFAULT_SCREEN_DPI : dpi) / float(USER_DEFAULT_SCREEN_DPI);

        // See if we have any global shortcuts that take priority over the context.
        if (!RmlWin32::ProcessKeyDownShortcuts(rmlContext, rml_key, rml_modifier, native_dp_ratio, true))
        {
            return 0;
        }

        rmlContext->ProcessKeyDown(RmlWin32::ConvertKey((int)wParam), RmlWin32::GetKeyModifierState());

        // The key was not consumed by the context either, try keyboard shortcuts of lower priority.
        if (!RmlWin32::ProcessKeyDownShortcuts(rmlContext, rml_key, rml_modifier, native_dp_ratio, false))
        {
            return 0;
        }
    }
    else if (msg == WM_KEYUP)
    {
        rmlContext->ProcessKeyUp(RmlWin32::ConvertKey((int)wParam), RmlWin32::GetKeyModifierState());
    }
    else if (msg == WM_CHAR)
    {
        static wchar_t first_u16_code_unit = 0;

        const wchar_t c = (wchar_t)wParam;
        Rml::Character character = (Rml::Character)c;

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
                Rml::String utf8 = StringUtils::wstos(std::wstring{ first_u16_code_unit, c });
                character = Rml::StringUtilities::ToCharacter(utf8.data(), utf8.data() + utf8.size());
            }
            else if (c == '\r')
            {
                // Windows sends new-lines as carriage returns, convert to endlines.
                character = (Rml::Character)'\n';
            }

            first_u16_code_unit = 0;

            // Only send through printable characters.
            if (((char32_t)character >= 32 || character == (Rml::Character)'\n') && character != (Rml::Character)127)
            {
                rmlContext->ProcessTextInput(character);
            }
        }
    }

    wndProcDetour.UnDetour();
    const auto result = wndProcDetour.GetOriginalFunc()(hWnd, msg, wParam, lParam);
    wndProcDetour.Detour(WndProc);

    return result;
}

bool RmlInterface::WinKeyDetour(const uint msg, const WPARAM wParam, const LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYUP:
        case WM_KEYDOWN:
            {
                auto el = rmlContext->GetFocusElement();
                if (!el || el->GetTagName() != "input")
                {
                    break;
                }

                auto attr = el->GetAttribute("type");
                if (!attr || attr->GetType() != attr->STRING)
                {
                    break;
                }

                auto attrValue = attr->Get<Rml::String>();
                if (attrValue == "text" || attrValue == "password")
                {
                    return false;
                }
            }
        default: break;
    }

    winKeyDetour.UnDetour();
    const auto result = winKeyDetour.GetOriginalFunc()(msg, wParam, lParam);
    winKeyDetour.Detour(WinKeyDetour);

    return result;
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
        bool down = currentMouseState[i];
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
            const int result = AddFontResourceExA(file.c_str(), FR_PRIVATE, nullptr);
            if (result != 0)
            {
                if (fonts.insert(file).second)
                {
                    Rml::LoadFontFace("file://" + file, ini.get_value_bool(1));
                }
            }
        }
    }

fallback:
    std::array<std::string, 9> systemFonts = {
        "segoeui.ttf ",  // Segoe UI (Latin; Greek; Cyrillic; Armenian; Georgian; Georgian Khutsuri; Arabic; Hebrew; Fraser)
        "tahoma.ttf ",   // Tahoma (Latin; Greek; Cyrillic; Armenian; Hebrew; Arabic; Thai)
        "meiryo.ttc ",   // Meiryo UI (Japanese)
        "msgothic.ttc",  // MS Gothic (Japanese)
        "msjh.ttc",      // Microsoft JhengHei (Chinese Traditional; Han; Han with Bopomofo)
        "msyh.ttc",      // Microsoft YaHei (Chinese Simplified; Han)
        "malgun.ttf ",   // Malgun Gothic (Korean)
        "simsun.ttc ",   // SimSun (Han Simplified)
        "seguiemj.ttf ", // Segoe UI (Latin; Greek; Cyrillic; Armenian; Georgian; Georgian Khutsuri; Arabic; Hebrew; Fraser)
    };

    for (auto& font : systemFonts)
    {
        std::array<char, MAX_PATH> windowsDir;
        auto len = GetSystemDirectoryA(windowsDir.data(), windowsDir.size());
        auto f = fonts.insert(std::format(R"(file://{}\{})", std::string_view(windowsDir.data(), len), font));
        if (f.second)
        {
            Rml::LoadFontFace(*f.first, true);
        }
    }
}

RmlInterface::RmlInterface(FlufUi* fluf, IDirect3D9* d3d9, IDirect3DDevice9* device)
    : ui(fluf), renderInterface(std::make_unique<RenderInterfaceDirectX9>(d3d9, device))
{
    module = this;
    winKeyDetour.Detour(WinKeyDetour);
    wndProcDetour.Detour(WndProc);

    // TODO: Support OpenGL as well
    systemInterface = std::make_unique<SystemInterface>();
    fileInterface = std::make_unique<FileInterface>();

    SetSystemInterface(systemInterface.get());
    SetFileInterface(fileInterface.get());
    SetRenderInterface(renderInterface.get());
    Rml::Initialise();
    // Rml::Lua::Initialise();

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
}

Rml::Context* RmlInterface::GetRmlContext() { return rmlContext; }
RmlInterface::~RmlInterface()
{
    Rml::Shutdown();
    module = nullptr;
}
