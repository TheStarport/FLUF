#include <PCH.hpp>

#include "FlightManualMarkdown.hpp"

#include "Utils/StringUtils.hpp"

FlightManualMarkdown::FlightManualMarkdown(ImGuiInterface* imguiInterface, std::function<void(std::string_view)> pageSelected)
    : ImguiMarkdown(imguiInterface), onPageSelected(std::move(pageSelected))
{
    parser.flags |= MD_FLAG_PERMISSIVEAUTOLINKS;
}

void FlightManualMarkdown::OpenUrl() const
{
    if (!href.starts_with("/"))
    {
        return;
    }

    using namespace std::string_view_literals;
    onPageSelected(StringUtils::ReplaceStr(href, "_"sv, " "sv));
}
