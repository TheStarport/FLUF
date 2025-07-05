#include "PCH.hpp"

#include "ShipDealerInterface.hpp"

#include "Vanilla/Components/FlButtonControl.hpp"
#include "Vanilla/Components/FlTextControl.hpp"
#include "Fluf.hpp"

ShipDealerInterface::ShipDealerInterface() : textControl(nullptr), shipDealerNextControl(nullptr), shipDealerPrevControl(nullptr), navBarControl(nullptr)
{
    initializedControls = false;
}
std::unordered_map<uint, std::vector<uint>>& ShipDealerInterface::GetBaseShipMap() { return orderedBaseShipMap; }
void ShipDealerInterface::SetCurrentShipPage(const uint value) { currentShipPage = value; }
uint ShipDealerInterface::GetCurrentShipPage() { return currentShipPage; }

void ShipDealerInterface::OnUpdate(FlControl* control)
{
    if (!initializedControls && control->GetName() == "NN_ShipTrader")
    {
        initializedControls = true;

        if (HasMoreThanThreeShips(shipPageCount))
        {
            shipDealerNextControl = this->CreateButtonControl(control,
                                                              "ShipTraderNextShips",
                                                              R"(interface\NEURONET\INVENTORY\inv_sellarrow.3db)",
                                                              nullptr,
                                                              "ui_select_item",
                                                              nullptr,
                                                              { 1.45f, -0.52f, -1.0f },
                                                              nullptr,
                                                              { 0.05f, 0.02f, 0.0f },
                                                              false,
                                                              false,
                                                              false,
                                                              false,
                                                              0,
                                                              0,
                                                              0);

            shipDealerPrevControl = this->CreateButtonControl(control,
                                                              "ShipTraderPrevShips",
                                                              R"(interface\NEURONET\INVENTORY\inv_buyarrow.3db)",
                                                              nullptr,
                                                              "ui_select_item",
                                                              nullptr,
                                                              { 1.30f, -0.52f, -1.0f },
                                                              nullptr,
                                                              { 0.05f, 0.02f, 0.0f },
                                                              false,
                                                              false,
                                                              false,
                                                              false,
                                                              0,
                                                              0,
                                                              0);

            textControl =
                this->CreateTextControl(control, "ShipTraderPageCount", "ItemName", { 0.271f, -0.047f, -1.0f }, nullptr, 0.1f, 0.1f, 1.0f, 0, 1, 0, true);
            const std::wstring textToDisplay = std::format(L"Page {}/{}", currentShipPage + 1, shipPageCount);
            textControl->SetTextValue(textToDisplay.c_str());
        }
    }

    if (!navBarControl && control->GetName() == "NavBar")
    {
        navBarControl = control;
    }
}
FlControl* ShipDealerInterface::NavBarFindShipDealer() const
{
    FlControl* child = navBarControl->GetChildControl();
    while (child)
    {
        if (*reinterpret_cast<uint*>(child) != 0x5e15f4) // check if vtable matches the ship dealer one
        {
            child = child->GetNextControl();
            continue;
        }
        if (const auto btn = reinterpret_cast<FlButtonControl*>(child); _stricmp(btn->iconFile, "interface\\baseside\\salesman.3db") == 0)
        {
            return child;
        }
        child = child->GetNextControl();
    }
    return nullptr;
}
bool ShipDealerInterface::OnExecuteButton(FlButtonControl* control)
{
    if (shipDealerNextControl && control->GetName() == "ShipTraderNextShips")
    {
        SetNextShips();
        control->GetParentControl()->FindChildControl("ShipTraderCancelButton")->Execute();

        if (const auto shipDealerBtn = NavBarFindShipDealer())
        {
            shipDealerBtn->Execute();
        }
    }
    else if (shipDealerPrevControl && control->GetName() == "ShipTraderPrevShips")
    {
        SetPrevShips();
        control->GetParentControl()->FindChildControl("ShipTraderCancelButton")->Execute();

        if (const auto shipDealerBtn = NavBarFindShipDealer())
        {
            shipDealerBtn->Execute();
        }
    }

    return true;
}
void ShipDealerInterface::OnTerminate(FlControl* control)
{
    if (const auto name = control->GetName(); name == "NN_ShipTrader")
    {
        initializedControls = false;

        shipDealerNextControl = nullptr;
        shipDealerPrevControl = nullptr;

        textControl = nullptr;
    }
    else if (name == "NavBar")
    {
        navBarControl = nullptr;
    }
}
