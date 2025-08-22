#include "PCH.hpp"

#include "FLUF.UI.hpp"
#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "PatchNote.hpp"
#include "PatchNoteConfig.hpp"
#include "PatchNotes.hpp"

#include "Exceptions.hpp"
#include "MemoryHelper.hpp"
#include "imgui_markdown.h"

#include "FLCore/Common/CommonMethods.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "Utils/StringUtils.hpp"

#include <curl/curl.h>

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

void PatchNotes::OnGameLoad()
{
    auto interface = flufUi->GetImGuiInterface();
    if (!interface)
    {
        Fluf::Log(LogLevel::Error, "PatchNotes cannot load, not in ImGui mode!");
        return;
    }

    interface->RegisterImGuiModule(this);

    patchNoteWindow = std::make_unique<PatchNoteWindow>(interface, &patches, &showFullNotes);

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
        if (!gamePaused)
        {
            MemoryHelper::PauseGame();
            gamePaused = true;
        }

        patchNoteWindow->Render();
    }
    else if (gamePaused)
    {
        MemoryHelper::UnPauseGame();
        gamePaused = false;
    }

    if (!MemoryHelper::IsInMainMenu() || patches.empty())
    {
        showFullNotes = false;
        return;
    }

    const auto interface = flufUi->GetImGuiInterface();

    auto& [date, content, preamble, version] = patches.front();

    constexpr float padding = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos({ (viewport->WorkPos.x + viewport->WorkSize.x - padding), (viewport->WorkPos.y + viewport->WorkSize.y - padding) },
                            ImGuiCond_Always,
                            ImVec2{ 1.0f, 1.0f });
    ImGui::SetNextWindowSize({ 400.f, 300.f }, ImGuiCond_Always);
    ImGui::Begin("Patch Notes##1", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    const auto itemSpacing = ImGui::GetStyle().ItemSpacing;
    auto* font = interface->GetImGuiFont("Saira", FontSize::Big);
    ImGui::PushFont(font);
    ImGui::Text(version.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize(date.str().c_str()).x + itemSpacing.x * 2.0f));
    ImGui::Text(date.str().c_str());
    ImGui::Separator();
    ImGui::PopFont();

    if (const auto buttonHeight = itemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("PreambleScrollingRegion", ImVec2(0.0f, -buttonHeight), ImGuiChildFlags_NavFlattened))
    {
        ImGui::TextWrapped(preamble.c_str());
    }

    ImGui::EndChild();
    ImGui::Separator();

    if (ImGui::Button("Read More"))
    {
        showFullNotes = true;
        patchNoteWindow->isOpen = true;
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
        throw ModuleLoadException("Patch Notes was loaded, but FLUF.UI's ui mode was not set to ImGui.");
    }

    config = std::make_shared<PatchNoteConfig>(*ConfigHelper<PatchNoteConfig, PatchNoteConfig::path>::Load());

    if (config->url.empty() || (!config->url.starts_with("https://") && !config->url.starts_with("http://")))
    {
        throw ModuleLoadException("No url was provided within the config file, or did not begin with http");
    }

    flufUi = module;
}

PatchNotes::~PatchNotes() { flufUi->GetImGuiInterface()->UnregisterImGuiModule(this); }

std::string_view PatchNotes::GetModuleName() { return moduleName; }

SETUP_MODULE(PatchNotes);
