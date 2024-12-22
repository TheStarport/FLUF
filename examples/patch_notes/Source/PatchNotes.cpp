#include "PCH.hpp"

#include "FLUF.UI/Include/FLUF.UI.hpp"
#include "FLUF/Include/Fluf.hpp"
#include "FLUF/Include/FlufModule.hpp"
#include "PatchNote.hpp"
#include "PatchNoteConfig.hpp"
#include "PatchNotes.hpp"
#include "imgui_markdown.h"

#include "FLCore/Common/CommonMethods.hpp"
#include "Utils/StringUtils.hpp"

#include <curl/curl.h>
#include <shellapi.h>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

struct MemoryBuffer
{
        char* data;
        size_t size;
};

size_t DownloadCallback(const void* ptr, const size_t size, const size_t nmemb, void* data)
{
    const size_t totalSize = size * nmemb;
    auto* mem = static_cast<MemoryBuffer*>(data);

    const auto newData = static_cast<char*>(realloc(mem->data, mem->size + totalSize + 1));
    if (newData == nullptr)
    {
        free(mem->data);
        mem->data = nullptr;

        // Memory allocation failed
        Fluf::Log(LogLevel::Error, "Not enough memory (realloc failed)");
        return 0;
    }

    mem->data = newData;
    memcpy(&mem->data[mem->size], ptr, totalSize);
    mem->size += totalSize;
    mem->data[mem->size] = 0; // Null-terminate the string

    return totalSize;
}

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

void PatchNotes::LoadPatchNotesFromCache(const std::string_view path)
{
    if (!std::filesystem::exists(path))
    {
        return;
    }

    const std::ifstream file(path.data(), std::ios::binary);
    if (!file.is_open())
    {
        Fluf::Log(LogLevel::Error, "Unable to open cache file for reading");
        return;
    }

    std::string buffer;
    std::copy(std::istreambuf_iterator(file.rdbuf()), std::istreambuf_iterator<char>(), std::back_inserter(buffer));

    const auto result = rfl::yaml::read<std::vector<PatchNote>>(buffer);
    if (result.error().has_value())
    {
        Fluf::Log(LogLevel::Error, std::format("Error reading payload: {}", result.error().value().what()));
        return;
    }
    patches = result.value();
    Fluf::Log(LogLevel::Info, "Loaded patch notes from local cache.");
}

void PatchNotes::RenderFullNotes()
{
    ImGui::SetNextWindowSize({ 1280.f, 1024.f }, ImGuiCond_Always);
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Patch Windows", &showFullNotes, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    for (const auto font = flufUi->GetImGuiFont("Saira", 32); auto& [date, content, preamble, version] : patches)
    {
        assert(font);

        ImGui::PushFont(font);
        ImGui::SeparatorText(version.c_str());
        ImGui::Text(date.str().c_str());
        ImGui::NewLine();
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

    ImGui::End();
}

void PatchNotes::OnGameLoad()
{
    std::array<char, MAX_PATH> totalPath{};
    GetUserDataPath(totalPath.data());

    const std::string cachePath = std::format("{}/patch_notes.yml", std::string(totalPath.data()));

    LoadPatchNotesFromCache(cachePath);

    if (config->url.empty() || (!config->url.starts_with("https://") && !config->url.starts_with("http://")))
    {
        return;
    }

    // Download latest crash payload, or load from the cache if not possible
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        Fluf::Log(LogLevel::Error, "Unable to initialise cURL");
        return;
    }

    // Reserve 1MB of space for our json payload.
    MemoryBuffer buffer{};
    curl_easy_setopt(curl, CURLOPT_URL, config->url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
    const CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        if (buffer.data)
        {
            free(buffer.data);
        }

        Fluf::Log(LogLevel::Error, std::format("Curl error: {}", curl_easy_strerror(res)));
        return;
    }

    const auto patchNotes = std::string(buffer.data, buffer.size);
    auto result = rfl::yaml::read<std::vector<PatchNote>>(patchNotes);
    free(buffer.data);
    if (result.error().has_value())
    {
        Fluf::Log(LogLevel::Error, std::format("Error reading payload: {}", result.error().value().what()));
        return;
    }

    patches = result.value();
    std::ofstream file(cachePath, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        Fluf::Log(LogLevel::Error, "Unable to open cache file for writing");
        return;
    }

    file.write(patchNotes.c_str(), patchNotes.size());
    file.close();
}

void PatchNotes::Render()
{
    if (showFullNotes)
    {
        RenderFullNotes();
    }

    static auto mainMenuControl = reinterpret_cast<PDWORD>(0x67BCC8);
    if (!*mainMenuControl || patches.empty())
    {
        return;
    }

    auto& firstPatch = patches.front();

    constexpr float padding = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos({ (viewport->WorkPos.x + viewport->WorkSize.x - padding), (viewport->WorkPos.y + viewport->WorkSize.y - padding) },
                            ImGuiCond_Always,
                            ImVec2{ 1.0f, 1.0f });
    ImGui::SetNextWindowSize({ 400.f, 300.f }, ImGuiCond_Always);
    ImGui::Begin("Patch Notes", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    auto* font = flufUi->GetImGuiFont("Saira", 32);
    ImGui::PushFont(font);
    ImGui::Text(firstPatch.version.c_str());
    ImGui::SameLine();
    ImGui::Text(firstPatch.date.str().c_str());
    ImGui::Separator();
    ImGui::PopFont();

    ImGui::Text(firstPatch.preamble.c_str());

    const auto remainingSpace = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosY(remainingSpace.y - 50);
    if (ImGui::Button("Read More"))
    {
        showFullNotes = true;
    }

    ImGui::End();
}

PatchNotes::PatchNotes()
{
    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        Fluf::Log(LogLevel::Error, "PatchNotes was loaded, but FLUF UI was not loaded. Crashes are likely.");
        return;
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    Fluf::Log(LogLevel::Trace, std::format("{}", reinterpret_cast<DWORD>(module.get())));
    if (module->GetConfig()->uiMode != UiMode::ImGui)
    {
        Fluf::Log(LogLevel::Error, "Patch Notes was loaded, but FLUF.UI's ui mode was not set to ImGui.");
        return;
    }

    config = std::make_shared<PatchNoteConfig>(ConfigHelper<PatchNoteConfig, PatchNoteConfig::path>::Load());

    if (config->url.empty() || (!config->url.starts_with("https://") && !config->url.starts_with("http://")))
    {
        Fluf::Log(LogLevel::Error, "No url was provided within the config file, or did not begin with http");
    }

    flufUi = module;
    flufUi->RegisterImGuiModule(this);
}

PatchNotes::~PatchNotes() { flufUi->UnregisterImGuiModule(this); }

std::string_view PatchNotes::GetModuleName() { return moduleName; }

SETUP_MODULE(PatchNotes);
