#include <PCH.hpp>

#include "FlightManualMarkdown.hpp"

#include "Utils/StringUtils.hpp"

#include <Fluf.hpp>
#include <KeyManager.hpp>
#include <utility>

bool FlightManualMarkdown::HandleKeyCode(std::string_view html)
{
    pugi::xml_document topLevelDoc;
    const auto result = topLevelDoc.load_buffer(html.data(), html.size(), pugi::parse_default | pugi::parse_fragment);
    if (result.status != pugi::status_ok)
    {
        return false;
    }

    const auto doc = topLevelDoc.document_element();
    const auto attr = doc.attribute("code");
    if (attr.empty())
    {
        return false;
    }

    const auto keyManager = Fluf::GetKeyManager();
    for (auto& keyMap = keyManager->GetKeyMap(); auto& key : keyMap)
    {
        if (key.name == std::string_view(attr.value()))
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

bool FlightManualMarkdown::HandleToolTip(std::string_view html, std::string_view nodeName, bool isEndNode)
{
    if (tooltipContent)
    {
        if (!isEndNode)
        {
            Fluf::Warn("Starting another tooltip without exiting a previous one is invalid!");
            return false;
        }

        ImGui::EndGroup();
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(tooltipContent->c_str());
        }

        tooltipContent = nullptr;
        return true;
    }

    pugi::xml_document topLevelDoc;
    const auto result = topLevelDoc.load_buffer(html.data(), html.size(), pugi::parse_default | pugi::parse_fragment);
    if (result.status != pugi::status_ok)
    {
        return false;
    }

    const auto doc = topLevelDoc.document_element();
    const auto attr = doc.attribute("id");

    if (attr.empty())
    {
        return false;
    }

    const auto tooltip = config->tooltips.value().find(attr.value());
    if (tooltip == config->tooltips.value().end())
    {
        return false;
    }

    tooltipContent = &tooltip->second;
    ImGui::BeginGroup();
    return true;
}

bool FlightManualMarkdown::CheckHtml(std::string_view html, std::string_view nodeName, bool isEndNode, bool isSelfClosing)
{
    // Due to how pugixml parses, it requires an end node to parse correctly,
    // but we do not care for the content in the middle, we just inject one here.
    // We will need to do this for anything that isn't self-closing
    const std::string completeNode = std::format("{}</{}>", html, nodeName);

    if (nodeName == "kc" || nodeName == "keycode")
    {
        return HandleKeyCode(html);
    }

    if (nodeName == "tooltip" || nodeName == "tt")
    {
        return HandleToolTip(completeNode, nodeName, isEndNode);
    }

    return ImguiMarkdown::CheckHtml(html, nodeName, isEndNode, isSelfClosing);
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

        const auto imageSize = ImVec2{ 200.f, 200.f };

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

void FlightManualMarkdown::DocumentEnd(const char* end)
{
    if (tooltipContent)
    {
        ImGui::EndGroup();
        tooltipContent = nullptr;
    }
}

FlightManualMarkdown::FlightManualMarkdown(ImGuiInterface* imguiInterface, rfl::Ref<FlightManualConfig> config,
                                           std::function<void(std::string_view)> pageSelected)
    : ImguiMarkdown(imguiInterface), onPageSelected(std::move(pageSelected)), config(std::move(config))
{
    parser.flags |= MD_FLAG_PERMISSIVEAUTOLINKS;
}
