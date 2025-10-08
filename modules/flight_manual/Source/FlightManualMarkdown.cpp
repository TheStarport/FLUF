#include <PCH.hpp>

#include "FlightManualMarkdown.hpp"

#include "Utils/StringUtils.hpp"

#include <Fluf.hpp>
#include <KeyManager.hpp>

bool FlightManualMarkdown::CheckHtml(const char* str, const char* str_end)
{
    std::string_view element = { str, str_end };
    constexpr std::string_view elStart = "<kc code=\"";
    constexpr std::string_view elEnd = "\">";

    if (!element.starts_with(elStart) || !element.ends_with(elEnd))
    {
        return ImguiMarkdown::CheckHtml(str, str_end);
    }

    std::string code;
    code.reserve(element.size());
    bool success = false;
    for (int i = elStart.size(); element[i] != '>'; i++)
    {
        if (element[i] == '"')
        {
            success = true;
            break;
        }

        code += element[i];
    }

    if (!success || code.size() <= 4)
    {
        return false;
    }

    const auto keyManager = Fluf::GetKeyManager();
    for (auto& keyMap = keyManager->GetKeyMap(); auto& key : keyMap)
    {
        if (key.name == code)
        {
            const auto currentKeyCode = KeyManager::TranslateKeyMapping(key);
            ImGui::Text("%s", currentKeyCode.c_str());
            return true;
        }
    }

    return false;
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

FlightManualMarkdown::FlightManualMarkdown(ImGuiInterface* imguiInterface, std::function<void(std::string_view)> pageSelected)
    : ImguiMarkdown(imguiInterface), onPageSelected(std::move(pageSelected))
{
    parser.flags |= MD_FLAG_PERMISSIVEAUTOLINKS;
}
