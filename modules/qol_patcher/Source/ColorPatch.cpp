#include "PCH.hpp"

#include "QolPatcher.hpp"

QolPatcher::ColorPatch::ColorPatch(const std::string& moduleName, const std::initializer_list<DWORD> offsets, DWORD* newColor, const bool isBgr,
                                   const bool includeAlpha)
    : MemoryPatch(moduleName, offsets, sizeof(DWORD), {}), newColor(newColor), bgr(isBgr), alpha(includeAlpha)
{
    const auto module = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName.empty() ? nullptr : moduleName.c_str()));
    if (!module)
    {
        return;
    }

    // Default to the original color if none provided
    if (!*newColor)
    {
        *newColor = *reinterpret_cast<DWORD*>(oldData.data());
        patchedData = oldData;
    }
    else
    {
        memcpy(patchedData.data(), newColor, sizeof(DWORD));
    }
}

void QolPatcher::ColorPatch::RenderComponent()
{
    DWORD color = *newColor;
    if (bgr)
    {
        // Swap the R/B values
        color = (color & 0xFF000000) | ((color & 0xFF0000) >> 16) | (color & 0x00FF00) | ((color & 0x0000FF) << 16);
    }

    ImVec4 colorArr = ImGui::ColorConvertU32ToFloat4(color);

    if (!alpha)
    {
        ImGui::ColorEdit3("##new-col", reinterpret_cast<float*>(&colorArr));
    }
    else
    {
        ImGui::ColorEdit4("##new-col", reinterpret_cast<float*>(&colorArr));
    }

    color = ImGui::ColorConvertFloat4ToU32(colorArr);

    if (bgr)
    {
        // Swap the R/B values
        color = (color & 0xFF000000) | ((color & 0xFF0000) >> 16) | (color & 0x00FF00) | ((color & 0x0000FF) << 16);
    }

    if (color != *newColor)
    {
        *newColor = color;
        memcpy(patchedData.data(), &color, sizeof(DWORD));
    }
}
