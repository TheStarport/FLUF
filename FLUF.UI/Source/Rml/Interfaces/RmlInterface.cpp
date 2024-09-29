#include "PCH.hpp"

#include <FLCore/FLCoreDefs.hpp>

#include "Rml/Interfaces/FileInterface.hpp"
#include "Rml/Interfaces/RenderInterfaceDirectX9.hpp"
#include "Rml/Interfaces/RmlInterface.hpp"

#include "Rml/Interfaces/SystemInterface.hpp"
#include <FLCore/Common/INI_Reader.hpp>

#include "RmlUi/Core/Core.h"
#include "RmlUi/Debugger/Debugger.h"
// #include "RmlUi/Lua.h"

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

            // Try add the font
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

    // No fonts loaded, load Times New Roman as an emergancy font
    if (fonts.empty())
    {
    fallback:
        const std::string timesNewRoman = R"(C:\Windows\Fonts\times.ttf)";
        Rml::LoadFontFace(std::string("file://") + timesNewRoman, true);
        fonts.insert(timesNewRoman);
    }
}

RmlInterface::RmlInterface(FlufUi* fluf, IDirect3D9* d3d9, IDirect3DDevice9* device)
    : ui(fluf), renderInterface(std::make_unique<RenderInterfaceDirectX9>(d3d9, device))
{
    // TODO: Support OpenGL as well
    systemInterface = std::make_unique<SystemInterface>();
    fileInterface = std::make_unique<FileInterface>();

    SetSystemInterface(systemInterface.get());
    SetFileInterface(fileInterface.get());
    SetRenderInterface(renderInterface.get());
    Rml::Initialise();
    // Rml::Lua::Initialise();

    const auto freelancerHwnd = FindWindowA(nullptr, "Freelancer");
    RECT rect;
    GetWindowRect(freelancerHwnd, &rect);

    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    rmlContext = Rml::CreateContext("main", Rml::Vector2i(width, height));
    LoadFonts();

    Rml::Debugger::Initialise(rmlContext);
#ifdef _DEBUG
    Rml::Debugger::SetVisible(true);
#endif
}
Rml::Context* RmlInterface::GetRmlContext() const { return rmlContext; }
RmlInterface::~RmlInterface() { Rml::Shutdown(); }
