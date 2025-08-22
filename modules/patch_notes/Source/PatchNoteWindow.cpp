#include <PCH.hpp>

#include "PatchNoteWindow.hpp"

#include "imgui_markdown.h"
#include "ImGui/IconFontAwesome6.hpp"
#include "ImGui/ImGuiInterface.hpp"

#include <shellapi.h>

static void MarkdownFormatCallback(const ImGui::MarkdownFormatInfo& markdownFormatInfo, bool start)
{
    // Call the default first so any settings can be overwritten by our implementation.
    // Alternatively could be called or not called in a switch statement on a case by case basis.
    // See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
    ImGui::defaultMarkdownFormatCallback(markdownFormatInfo, start);
}

// ReSharper disable once CppPassValueParameterByConstReference
static void LinkCallback(ImGui::MarkdownLinkCallbackData data)
{
    const std::string url(data.link, data.linkLength);
    if (!url.starts_with("http://") || url.find(';') != std::string::npos || url.find("&&") != std::string::npos || url.find(' ') != std::string::npos)
    {
        return;
    }

    if (!data.isImage)
    {
        ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}

void PatchNoteWindow::RenderWindowContents()
{
    if (ImGui::Button(ICON_FA_ARROW_LEFT "##fo-go-back"))
    {
        isOpen = false;
        *openState = false;
        return;
    }

    isOpen = *openState;
    for (const auto font = interface->GetImGuiFont("Saira", FontSize::Big); auto& [date, content, preamble, version] : *patches)
    {
        assert(font);

        ImGui::PushFont(font);
        ImGui::SeparatorText(version.c_str());
        ImGui::Text(date.str().c_str());
        ImGui::PopFont();

        // clang-format off
        static constexpr ImGui::MarkdownConfig markdownConfig = ImGui::MarkdownConfig{
            .linkCallback = LinkCallback,
            .imageCallback = nullptr,
            .linkIcon = nullptr,
            .userData = nullptr,
            .formatCallback = MarkdownFormatCallback
        };
        // clang-format on

        Markdown(content.c_str(), content.size(), markdownConfig);
    }
}

PatchNoteWindow::PatchNoteWindow(ImGuiInterface* interface, std::vector<PatchNote>* patches, bool* openState)
    : FlWindow("Patch Notes", ImGuiWindowFlags_NoResize), patches(patches), interface(interface), openState(openState)
{
    CenterWindow();
    SetSize(ImVec2(1024.f, 1024.f));
}
