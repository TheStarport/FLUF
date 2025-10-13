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
            auto currentKeyCode = KeyManager::TranslateKeyMapping(key);
            if (currentKeyCode.empty())
            {
                currentKeyCode = "[UNBOUND]";
            }

            ImGui::PushStyleColor(ImGuiCol_Text, 0xB00899FC);
            RenderText(currentKeyCode.c_str(), currentKeyCode.c_str() + currentKeyCode.size());
            ImGui::PopStyleColor();
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

bool FlightManualMarkdown::GetImage(image_info& nfo) const
{
    if (const auto success = ImguiMarkdown::GetImage(nfo); !success)
    {
        constexpr std::string_view badPath = "IMAGE NOT FOUND";
        const auto badImageSize = ImGui::CalcTextSize(badPath.data());

        auto imageSize = ImVec2{ 200.f, 200.f };

        auto textPos = ImGui::GetCursorScreenPos();
        ImGui::ImageWithBg({}, imageSize, {}, {}, { 1.f, 1.f, 1.f, 1.f });
        auto nextElPos = ImGui::GetCursorScreenPos();

        textPos += imageSize * 0.5f;
        textPos.x -= badImageSize.x * 0.5f;
        ImGui::SetCursorScreenPos(textPos);
        ImGui::Text(badPath.data());

        ImGui::SetCursorScreenPos({ nextElPos.x, nextElPos.y + badImageSize.y });
        return false;
    }

    return true;
}

FlightManualMarkdown::FlightManualMarkdown(ImGuiInterface* imguiInterface, std::function<void(std::string_view)> pageSelected)
    : ImguiMarkdown(imguiInterface), onPageSelected(std::move(pageSelected))
{
    parser.flags |= MD_FLAG_PERMISSIVEAUTOLINKS;
}
