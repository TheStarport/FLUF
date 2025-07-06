#include "PCH.hpp"

#include "SetInfo.hpp"

#include "ClientServerCommunicator.hpp"
#include "FLUF.UI.hpp"
#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "Exceptions.hpp"

#include "imgui_markdown.h"
#include "TextEditor.h"
#include "imgui_internal.h"
#include "ImGui/IconFontAwesome6.hpp"

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void SetInfo::OnGameLoad()
{
    if (flufUi->GetConfig()->uiMode != UiMode::ImGui)
    {
        return;
    }

    const auto imgui = flufUi->GetImGuiInterface();

    imgui->RegisterImGuiModule(this);

    // TODO: Standardise font sizes!
    const auto h1 = imgui->GetDefaultFont(48);
    const auto h2 = imgui->GetDefaultFont(46);
    const auto h3 = imgui->GetDefaultFont();

    // clang-format off
    mdConfig = {
        .linkCallback = nullptr,
        .tooltipCallback = nullptr,
        .imageCallback = nullptr,
        .linkIcon = ICON_FA_LINK,
        .userData = nullptr,
        .formatCallback = ImGui::defaultMarkdownFormatCallback
    };
    // clang-format on

    mdConfig.headingFormats[0] = { const_cast<ImFont*>(h1), true };
    mdConfig.headingFormats[1] = { const_cast<ImFont*>(h2), true };
    mdConfig.headingFormats[2] = { const_cast<ImFont*>(h3), true };

    editor = std::make_unique<TextEditor>();
}

void SetInfo::Render()
{
    if (!renderInfoCardEditPage)
    {
        return;
    }

    static ImVec2 windowSize{ 1280.f, 1024.f };
    static ImVec2 childWindowSize{ windowSize.x - 20.f, windowSize.y * 0.45f };
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Infocard Editor", &renderInfoCardEditPage, ImGuiWindowFlags_NoResize);

    ImGui::SetNextWindowSize(childWindowSize, ImGuiCond_Always);
    ImGui::BeginChild("infocard-preview-markdown");
    ImGui::Markdown(infocardBeingEdited.c_str(), infocardBeingEdited.size(), mdConfig);
    ImGui::EndChild();

    ImGui::SetNextWindowSize(childWindowSize, ImGuiCond_Always);
    ImGui::BeginChild("infocard-edit-markdown");
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

    editor->Render("Infocard Editor");

    ImGui::PopFont();
    ImGui::EndChild();

    infocardBeingEdited = editor->GetText();

    ImGui::BeginDisabled(waitingForUpdate);
    if (ImGui::Button("Save Changes"))
    {
        SendServerInfocardUpdate();
    }
    ImGui::EndDisabled();

    ImGui::End();
}

FlufModule::ModuleProcessCode SetInfo::OnPayloadReceived(uint sourceClientId, const FlufPayload& payload)
{
    if (strncmp(payload.header, "sinf", sizeof(payload.header)) != 0)
    {
        return ModuleProcessCode::ContinueUnhandled;
    }

    waitingForUpdate = false;

    auto infocards = payload.Convert<std::unordered_map<uint, std::string>>();
    if (infocards.error())
    {
        Fluf::Error(std::format("Received infocard update packet, but was malformed: \n{}", infocards.error().value().what()));
        return ModuleProcessCode::Handled;
    }

    for (auto& [client, infocard] : infocards.value())
    {
        clientInfocards[client] = infocard;
    }

    return ModuleProcessCode::Handled;
}

void SetInfo::SendServerInfocardUpdate()
{
    constexpr char header[4] = { 's', 'i', 'n', 'f' };
    auto payload = FlufPayload::ToPayload(infocardBeingEdited, header);
    Fluf::GetClientServerCommunicator()->SendPayloadFromClient(header, true);
    waitingForUpdate = true;
}

SetInfo::SetInfo()
{
    AssertRunningOnClient;

    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        throw ModuleLoadException("SetInfo was loaded, but FLUF UI was not loaded.");
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    flufUi = module;

    if (module->GetConfig()->uiMode != UiMode::ImGui)
    {
        throw ModuleLoadException("SetInfo was loaded, but FLUF UI's ui mode was not set to 'ImGui'");
    }
}

SetInfo::~SetInfo()
{
    if (flufUi->GetConfig()->uiMode == UiMode::ImGui)
    {
        flufUi->GetImGuiInterface()->UnregisterImGuiModule(this);
    }
}

std::string_view SetInfo::GetModuleName() { return moduleName; }

SETUP_MODULE(SetInfo);
