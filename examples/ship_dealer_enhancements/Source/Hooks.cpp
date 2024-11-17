#include "PCH.hpp"
#include "ShipDealerEnhancements.hpp"

#include "FLCore/Common/CommonMethods.hpp"
#include "FLCore/Common/Unknown.hpp"
#include "Utils/Detour.hpp"
#include "Utils/MemUtils.hpp"

using RenderShipListFunc = bool(__fastcall*)(void* ShipDealer, void* edx, int a1, int a2);
FunctionDetour renderShipListDetour(reinterpret_cast<RenderShipListFunc>(0x4B7440));

struct ShipListEntry
{
        ShipListEntry* next;
        ShipListEntry* prev;
        uint* shipArchId;
};

struct ShipList
{
        uint allocator;
        ShipListEntry* firstEntry;
        uint shipCounter;
};

static std::vector<uint*> shipsToShow;
static FlMap<uint, MarketGoodInfo>* mappy;

uint __fastcall GetShipListDetour(void* baseInfo, void* edx, ShipList* sf, int searchedType)
{
    for (uint* shipId : shipsToShow)
    {
        ShipListEntry* firstNode = sf->firstEntry;
        ShipListEntry* lastNode = firstNode->prev;

        auto* newNode = static_cast<ShipListEntry*>(malloc(0xC));
        newNode->next = firstNode; // new element next = start
        newNode->prev = lastNode;

        lastNode->next = newNode;
        firstNode->prev = newNode;

        newNode->shipArchId = shipId;
        sf->shipCounter++;
    }
    return sf->shipCounter;
}

const static uint* currentBase = reinterpret_cast<uint*>(0x66873C);
static void* lastShipDealer = nullptr;
static int currBaseShipCount;

void ReloadShipCount()
{
    currBaseShipCount = 0;

    BaseData* base_data = BaseDataList_get()->get_base_data(*currentBase);
    mappy = reinterpret_cast<FlMap<uint, MarketGoodInfo>*>(&base_data->marketMap);

    for (auto marketGood = mappy->begin(); marketGood != mappy->end() && marketGood.key() != 0; ++marketGood)
    {
        if (const auto type = GoodList::find_by_id(marketGood.value()->goodId)->type; type != GoodType::Ship || !marketGood.value()->stock)
        {
            continue;
        }
        currBaseShipCount++;
    }
}

bool CheckShipOverflow()
{
    ReloadShipCount();
    if (ShipDealerInterface::GetCurrentShipPage() > (currBaseShipCount - 1) / 3)
    {
        ShipDealerInterface::SetCurrentShipPage(0);
        return false;
    }
    return true;
}

bool HasMoreThanThreeShips(uint& pageCount)
{
    CheckShipOverflow();
    if (currBaseShipCount > 3)
    {
        pageCount = (currBaseShipCount - 1) / 3 + 1;
        return true;
    }

    return false;
}

void SetNextShips()
{
    uint shipPage = ShipDealerInterface::GetCurrentShipPage();
    ShipDealerInterface::SetCurrentShipPage(++shipPage);
    CheckShipOverflow();
}

void SetPrevShips()
{
    uint shipPage = ShipDealerInterface::GetCurrentShipPage();
    if (shipPage)
    {
        shipPage--;
    }
    else
    {
        shipPage = (currBaseShipCount - 1) / 3;
    }
    ShipDealerInterface::SetCurrentShipPage(shipPage);
}

bool GetOrderedShips(const uint page)
{
    ReloadShipCount();

    if (const auto baseShipIter = ShipDealerInterface::GetBaseShipMap().find(*currentBase);
        baseShipIter != ShipDealerInterface::GetBaseShipMap().end() && !baseShipIter->second.empty() && currBaseShipCount == baseShipIter->second.size())
    {
        shipsToShow.clear();
        const uint lastItem = std::min((page + 1) * 3, baseShipIter->second.size());

        for (uint i = page * 3; i < lastItem; i++)
        {
            shipsToShow.push_back(&baseShipIter->second.at(i));
        }

        return true;
    }

    return false;
}

bool __fastcall RenderShipList(void* ShipDealer, void* edx, const int a1, const int a2)
{
    static uint lastBase = *currentBase;

    if (lastBase != *currentBase)
    {
        ShipDealerInterface::SetCurrentShipPage(0);
        lastBase = *currentBase;
    }

    std::vector<uint*> firstThreeShips;
    if (const auto shipPage = ShipDealerInterface::GetCurrentShipPage(); !GetOrderedShips(shipPage))
    {
        int counter = 0;

        lastShipDealer = ShipDealer;

        BaseData* base_data = BaseDataList_get()->get_base_data(*currentBase);
        mappy = reinterpret_cast<FlMap<uint, MarketGoodInfo>*>(&base_data->marketMap);

        shipsToShow.clear();

        for (auto marketGood = mappy->begin(); marketGood != mappy->end() && marketGood.key() != 0; ++marketGood)
        {
            if (const auto type = GoodList::find_by_id(marketGood.value()->goodId)->type; type != GoodType::Ship || !marketGood.value()->stock)
            {
                continue;
            }

            if (counter < 3)
            {
                firstThreeShips.push_back(&marketGood.value()->goodId);
            }

            if (counter >= shipPage * 3 && counter <= (shipPage * 3) + 2)
            {
                shipsToShow.push_back(&marketGood.value()->goodId);
            }
            counter++;
        }
    }

    if (shipsToShow.empty())
    {
        ShipDealerInterface::SetCurrentShipPage(0);
        shipsToShow = firstThreeShips;
    }

    renderShipListDetour.UnDetour();
    const bool retVal = renderShipListDetour.GetOriginalFunc()(ShipDealer, edx, a1, a2);
    renderShipListDetour.Detour(RenderShipList);

    return retVal;
}

void __cdecl FreeShipData(void* shipData) { free(shipData); }

void ShipDealerEnhancements::InitShipDealerHooks()
{
    const auto freelancerHandle = GetModuleHandle(nullptr);

    renderShipListDetour.Detour(RenderShipList);

    MemUtils::PatchCallAddr(freelancerHandle, 0xB74AC, GetShipListDetour);
    MemUtils::PatchCallAddr(freelancerHandle, 0x77C4B, GetShipListDetour);
    MemUtils::PatchCallAddr(freelancerHandle, 0xB85AA, FreeShipData);
    MemUtils::PatchCallAddr(freelancerHandle, 0x77CB4, FreeShipData);
}
