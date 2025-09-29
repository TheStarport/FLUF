#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/Detour.hpp"
#include "Utils/MemUtils.hpp"
#include "Utils/StringUtils.hpp"

// Replace the table address with our own.
#define ADDR_SHIPCLASS1  ((PDWORD)(0x4b79ca + 3))
#define ADDR_SHIPCLASS2  ((PDWORD)(0x4b808d + 3))
#define ADDR_SHIPCLASS3  ((PDWORD)(0x4b8be7 + 3))
#define ADDR_SHIPCLASS4  ((PDWORD)(0x4b8fd2 + 3))
#define ADDR_SHIPCLASS5  ((PDWORD)(0x4b95b1 + 3))

// Increase the count.
#define ADDR_SHIPCLASSN1 ((PBYTE)(0x4b7682 + 2))
#define ADDR_SHIPCLASSN2 ((PBYTE)(0x4b807c + 2))

std::unordered_map<uint, std::string> shipClasses{
    {  0,      "Light Fighter" },
    {  1,      "Heavy Fighter" },
    {  2,          "Freighter" },
    {  3, "Very Heavy Fighter" },
    {  4,            "Gunboat" },
    {  5,            "Cruiser" },
    {  6,          "Destroyer" },
    {  7,         "Battleship" },
    {  8,            "Capital" },
    {  9,          "Transport" },
    { 10,    "Large Transport" },
    { 11,              "Train" },
    { 12,        "Large Train" },
};

std::array<uint, 100> shipIdsTable;

using LoadResourceDll = bool (*)(const char*);
using ForceLoadResource = DWORD (*)(HINSTANCE);
std::unique_ptr<FunctionDetour<LoadResourceDll>> loadResourceDllDetour;
HINSTANCE thisDll;

bool ResourceDllLoadDetour(const char* unk)
{
    loadResourceDllDetour->UnDetour();
    const auto res = loadResourceDllDetour->GetOriginalFunc()(unk);

    const auto resourceNumber = *reinterpret_cast<PDWORD>(0x67c404) - *reinterpret_cast<PDWORD>(0x67c400) << 14;
    for (unsigned int& i : shipIdsTable)
    {
        i += resourceNumber;
    }

    const auto loadResourceFunc = reinterpret_cast<ForceLoadResource>(0x57d800);
    loadResourceFunc(thisDll);

    loadResourceDllDetour->Detour(ResourceDllLoadDetour);
    return res;
}

void Fixes::AddNewShipClasses(HINSTANCE dll)
{
    thisDll = dll;

    static constexpr std::string_view configName = "modules/config/ship_classes.yml";
    using ConfigHelperType = ConfigHelper<std::unordered_map<uint, std::string>>;

    if (const auto classes = ConfigHelperType::Load(configName, false, false); classes.has_value())
    {
        shipClasses = classes.value();
    }
    else
    {
        if (MessageBoxA(nullptr,
                        "Ship classes not defined or malformed. Would you like to generate default ship classes?\n"
                        "Note: This will delete modules/config/ship_classes.yml, if it already exists.",
                        "No ship classes",
                        MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
        {
            std::abort();
        }

        ConfigHelperType::Save(configName, shipClasses);
    }
    // Create classes 0-99
    for (uint i = 0; i < shipIdsTable.size(); i++)
    {
        shipIdsTable[i] = i;

        auto cls = shipClasses.find(i);
        if (cls == shipClasses.end())
        {
            continue;
        }

        if (cls->second.size() > 64)
        {
            MessageBoxA(nullptr, std::format("Ship classes ({}) must contain no more than 64 characters.", cls->second).c_str(), "Too many characters", MB_OK);
            continue;
        }

        constexpr size_t len = 64; // (64 dash characters, in wide string form)
        wchar_t* buffer;
        // Returns a read-only pointer to the buffer of size len
        LoadStringW(dll, i, reinterpret_cast<LPWSTR>(&buffer), 0);

        // Make the buffer writable
        MemUtils::Protect(buffer, len, false);

        std::wstring className = StringUtils::stows(cls->second);
        memset(buffer, 0, len);                                        // Write null terminators into every slot
        memcpy_s(buffer, len, className.data(), className.size() * 2); // Copy all the string we need]
    }

    loadResourceDllDetour = std::make_unique<FunctionDetour<LoadResourceDll>>(reinterpret_cast<LoadResourceDll>(0x5B0C30));
    loadResourceDllDetour->Detour(ResourceDllLoadDetour);

    MemUtils::Protect(ADDR_SHIPCLASSN1, 1);
    MemUtils::Protect(ADDR_SHIPCLASSN2, 1);
    MemUtils::Protect(ADDR_SHIPCLASS1, 4);
    MemUtils::Protect(ADDR_SHIPCLASS2, 4);
    MemUtils::Protect(ADDR_SHIPCLASS3, 4);
    MemUtils::Protect(ADDR_SHIPCLASS4, 4);
    MemUtils::Protect(ADDR_SHIPCLASS5, 4);

    *ADDR_SHIPCLASS1 = *ADDR_SHIPCLASS2 = *ADDR_SHIPCLASS3 = *ADDR_SHIPCLASS4 = *ADDR_SHIPCLASS5 = reinterpret_cast<DWORD>(shipIdsTable.data());
    *ADDR_SHIPCLASSN1 = *ADDR_SHIPCLASSN2 = shipIdsTable.size();
}
