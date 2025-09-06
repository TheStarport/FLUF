#pragma once

#include <imgui.h>
#include <stack>
#include <string>
#include <type_traits>

class FlWindow;
using OnBreadcrumbItemClicked = void (FlWindow::*)(std::string_view page);
class Breadcrumb
{
        std::vector<std::string> items;
        FlWindow* module;
        OnBreadcrumbItemClicked onBreadcrumbItemClicked;

        static bool ArrowButton(ImVec2 pos, ImVec2 size, const char* id, std::string_view text, bool lastItem);

    public:
        Breadcrumb(FlWindow* module, OnBreadcrumbItemClicked onClickCallback, std::string_view root);
        void AddItem(std::string_view text);
        void Reset();
        void Render();
};
