#include <PCH.hpp>

#include "FlightManualMarkdown.hpp"

#include "Utils/StringUtils.hpp"

#include <Fluf.hpp>
#include <KeyManager.hpp>
#include <utility>
#include <ImGui/ImGuiInterface.hpp>
#include <ImGui/Fonts/IconFontAwesome6.hpp>

bool FlightManualMarkdown::HandleKeyCode(const std::string_view html)
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

bool FlightManualMarkdown::HandleToolTip(const std::string_view html, const bool isEndNode)
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

bool FlightManualMarkdown::HandleInfoBox(std::string_view html, const bool isEndNode)
{
    suppressingText = !isEndNode;
    if (isEndNode)
    {
        return true;
    }

    pugi::xml_document topLevelDoc;
    const auto result = topLevelDoc.load_buffer(html.data(), html.size(), pugi::parse_default | pugi::parse_fragment);
    if (result.status != pugi::status_ok)
    {
        return false;
    }

    auto node = topLevelDoc.document_element();
    auto backColor = 0xFF808080;
    auto stripeColor = 0xFFA0A0A0;
    auto textColor = ImGui::GetColorU32(ImGuiCol_Text);

#define HANDLE_COLOR(str, var)                                                                                         \
    if (auto attr = node.attribute(str); !attr.empty() && strlen(attr.as_string()) == 7 && attr.as_string()[0] == '#') \
    {                                                                                                                  \
        auto num = strtoul(attr.as_string() + 1, nullptr, 16);                                                         \
        const int r = (int)(num >> IM_COL32_R_SHIFT) & 0xFF;                                                           \
        const int g = (int)(num >> IM_COL32_G_SHIFT) & 0xFF;                                                           \
        const int b = (int)(num >> IM_COL32_B_SHIFT) & 0xFF;                                                           \
        var = r | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (0xFF << IM_COL32_A_SHIFT);                      \
    }

    HANDLE_COLOR("background", backColor);
    HANDLE_COLOR("text", textColor);
    HANDLE_COLOR("stripe", stripeColor);

    std::string icon;
    std::string title;
    if (auto attr = node.attribute("icon"); !attr.empty())
    {
        if (_strcmpi("bell", attr.as_string()) == 0)
        {
            icon = ICON_FA_BELL "  ";
        }
        else if (_strcmpi("radiation", attr.as_string()) == 0)
        {
            icon = ICON_FA_RADIATION "  ";
        }
        else if (_strcmpi("triangle-exclamation", attr.as_string()) == 0)
        {
            icon = ICON_FA_TRIANGLE_EXCLAMATION "  ";
        }
        else if (_strcmpi("skull-crossbones", attr.as_string()) == 0)
        {
            icon = ICON_FA_SKULL_CROSSBONES "  ";
        }
        else if (_strcmpi("circle-exclamation", attr.as_string()) == 0)
        {
            icon = ICON_FA_CIRCLE_EXCLAMATION "  ";
        }
        else if (_strcmpi("book", attr.as_string()) == 0)
        {
            icon = ICON_FA_BOOK "  ";
        }
        else if (_strcmpi("book-open", attr.as_string()) == 0)
        {
            icon = ICON_FA_BOOK_OPEN "  ";
        }
    }

    if (auto attr = node.attribute("title"); !attr.empty())
    {
        title = attr.as_string();
    }

    float titleSize = 0.0f;
    if (auto attr = node.attribute("title_size"); !attr.empty())
    {
        titleSize = attr.as_float();
    }

    auto possibleWidth = ImGui::GetContentRegionAvail().x * 0.8f;
    auto currentPos = ImGui::GetCursorScreenPos();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, backColor);
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    ImGui::SetCursorScreenPos({ currentPos.x + possibleWidth * 0.1f, currentPos.y });
    ImGui::BeginChild(std::format("##infobox-{}", infoboxId++).c_str(), { possibleWidth, 0.f }, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);

    if (!icon.empty())
    {
        ImGui::Text(icon.c_str());
    }

    if (!title.empty())
    {
        if (!icon.empty())
        {
            ImGui::SameLine();
        }

        ImGui::PushFont(imguiInterface->GetDefaultFont(FontStyle::BoldStyle), titleSize == 0.0f ? FontSize::Big : titleSize);
        ImGui::Text(title.c_str());
        ImGui::PopFont();
    }

    ImGui::Text(node.child_value());

    ImGui::EndChild();
    ImGui::PopStyleColor(2);

    return true;
}

bool FlightManualMarkdown::CheckHtml(const std::string_view html, const std::string_view nodeName, const bool isEndNode, const bool isSelfClosing)
{
    if (nodeName == "kc" || nodeName == "keycode")
    {
        return HandleKeyCode(html);
    }

    if (nodeName == "tooltip" || nodeName == "tt")
    {
        return HandleToolTip(html, isEndNode);
    }

    if (nodeName == "infobox" || nodeName == "info")
    {
        return HandleInfoBox(html, isEndNode);
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

void FlightManualMarkdown::RenderText(const char* str, const char* str_end)
{
    if (currentlyInInlineImage && ImGui::GetCursorScreenPos().y >= desiredEndHeight)
    {
        currentlyInInlineImage = false;
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    ImguiMarkdown::RenderText(str, str_end);
}

bool FlightManualMarkdown::GetImage(image_info& nfo)
{
    if (currentlyInInlineImage)
    {
        currentlyInInlineImage = false;
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    if (const auto success = ImguiMarkdown::GetImage(nfo); success)
    {
        return true;
    }

    constexpr std::string_view badPath = "IMAGE NOT FOUND";
    const auto badImageSize = ImGui::CalcTextSize(badPath.data());

    constexpr auto imageSize = ImVec2{ 200.f, 200.f };

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

void FlightManualMarkdown::RenderImage(image_info& nfo)
{
    if (nfo.title.find("#inline") == std::string_view::npos)
    {
        ImguiMarkdown::RenderImage(nfo);
        return;
    }

    auto space = ImGui::GetContentRegionAvail();
    const float ratio = nfo.size.y / nfo.size.x;
    nfo.size = { space.x * 0.3f, space.y };
    nfo.size.y = nfo.size.x * ratio;

    auto currentPos = ImGui::GetCursorScreenPos();
    ImGui::BeginGroup();

    ImGui::Image(nfo.textureId, nfo.size, nfo.uv0, nfo.uv1, nfo.colTint, nfo.colBorder);

    const auto newPos = ImVec2{ currentPos.x + currentPos.x * 0.2f, currentPos.y - nfo.size.y * 0.5f };
    ImGui::SetCursorScreenPos(newPos);
    ImGui::BeginChild("InlineImage##", { space.x * 0.68f, 0.0f }, ImGuiChildFlags_AutoResizeY);
    desiredEndHeight = newPos.y + nfo.size.y;
    currentlyInInlineImage = true;
}

void FlightManualMarkdown::DocumentEnd(const char* end)
{
    if (currentlyInInlineImage)
    {
        currentlyInInlineImage = false;
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    infoboxId = 0;

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
