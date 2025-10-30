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

void QolPatcher::ColorPatch::ReturnShieldState(CEqObj* eqObj, float& maxHP, float& currHP, bool& shieldUp)
{
    currHP = 0.0f;
    maxHP = 0.0f;
    CEquipTraverser tr((int)EquipmentClass::Shield, true);
    CEShield* shield = nullptr;
    while (shield = reinterpret_cast<CEShield*>(eqObj->equipManager.Traverse(tr)))
    {

        if (!config->showInactiveShield && !shield->IsFunctioning())
        {
            continue;
        }

        if (config->showInactiveShield)
        {
            bool isShieldGenActive = false;
            for (auto& shieldGen : shield->linkedShieldGen)
            {
                if (shieldGen->isActive)
                {
                    isShieldGenActive = true;
                    break;
                }
            }
            DWORD newShieldColor;
            
            if (!isShieldGenActive)
            {
                newShieldColor = config->frozenShieldBarColor;
            }
            else if (shield->IsFunctioning())
            {
                if (config->customShieldBarColor)
                {
                    newShieldColor = config->newShieldColor;
                }
                else
                {
                    newShieldColor = *reinterpret_cast<DWORD*>(ColorPatch::shieldColorPatch->oldData.data());
                }
            }
            else
            {
                newShieldColor = config->inactiveShieldColor;
            }

            static DWORD shieldColorAddr = DWORD(GetModuleHandleA(nullptr)) + 0x0D5843;
            MemUtils::WriteProcMem(shieldColorAddr, &newShieldColor, sizeof(DWORD));
        }

        float offlineThresholdSize = shield->GetMaxHitPoints() * shield->GetOfflineThreshold();
        float maxActiveHP = std::max(0.0f, shield->GetMaxHitPoints() - offlineThresholdSize);
        float currActiveHP = std::max(0.0f, shield->GetHitPoints() - offlineThresholdSize);

        maxHP += maxActiveHP;
        currHP += currActiveHP;
    }

}

void QolPatcher::RegisterColorPatches() { MemUtils::PatchCallAddr(GetModuleHandleA(nullptr), 0xD57FB, (void*)ColorPatch::ReturnShieldState); }
