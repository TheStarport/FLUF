#include <PCH.hpp>

#include "FlightManualMarkdown.hpp"

#include "Utils/StringUtils.hpp"

bool FlightManualMarkdown::CheckHtml(const char* str, const char* str_end)
{
    const size_t sz = str_end - str;
    constexpr std::string_view elStart = "<kc code=\"";
    constexpr std::string_view elEnd = "\">";

    if (strcmp(str, elStart.data()) != 0)
    {
        return ImguiMarkdown::CheckHtml(str, str_end);
    }

    std::string code;
    code.reserve(sz);
    bool success = false;
    for (int i = elStart.size(); i < sz && str[i] != '>'; i++)
    {
        code += str[i];
        if (i + 1 == '"')
        {
            success = true;
            break;
        }
    }

    if (!success || code.size() <= 4 || strcmp(str + code.size(), elEnd.data()) != 0)
    {
        return false;
    }

    return true;
}

void FlightManualMarkdown::HtmlDiv(const std::string& dclass, bool e) { ImguiMarkdown::HtmlDiv(dclass, e); }

void FlightManualMarkdown::OpenUrl() const
{
    if (!href.starts_with("/"))
    {
        return;
    }

    using namespace std::string_view_literals;
    onPageSelected(StringUtils::ReplaceStr(href, "_"sv, " "sv));
}

FlightManualMarkdown::FlightManualMarkdown(ImGuiInterface* imguiInterface, std::function<void(std::string_view)> pageSelected)
    : ImguiMarkdown(imguiInterface), onPageSelected(std::move(pageSelected))
{
    parser.flags |= MD_FLAG_PERMISSIVEAUTOLINKS;
}
