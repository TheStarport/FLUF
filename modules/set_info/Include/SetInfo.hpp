#pragma once

#include "FlufModule.hpp"
#include "ImGui/ImGuiModule.hpp"

#include "imgui_markdown.h"
#include "TextEditor.h"

#include <memory>

class FlufUi;

class SetInfo final : public FlufModule
{
        std::shared_ptr<FlufUi> flufUi;
        std::unordered_map<uint, std::string> clientInfocards;
        std::string infocardBeingEdited;
        ImGui::MarkdownConfig mdConfig;
        bool waitingForUpdate = false;
        std::unique_ptr<TextEditor> editor;

        bool renderInfoCardEditPage = false;

        void OnGameLoad() override;
        void Render();
        ModuleProcessCode OnPayloadReceived(uint sourceClientId, const FlufPayload& payload) override;
        void SendServerInfocardUpdate();

    public:
        static constexpr std::string_view moduleName = "Set Info";

        SetInfo();
        ~SetInfo() override = default;
        std::string_view GetModuleName() override;
};
