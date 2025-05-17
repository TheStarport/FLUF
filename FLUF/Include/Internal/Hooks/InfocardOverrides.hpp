#pragma once

#include "Utils/Detour.hpp"

struct FlufPayload;
class ClientServerCommunicator;
class InfocardOverrides
{
        friend ClientServerCommunicator;
        InfocardOverrides() = delete;

        inline static std::unordered_map<uint, std::wstring> infoCardOverrides;
        inline static std::unordered_map<uint, std::wstring> infoNameOverrides;

        using GetIdsName = int (*)(void* resourceHandle, uint ids, wchar_t* buffer, int length);
        inline static std::unique_ptr<FunctionDetour<GetIdsName>> getIdsNameDetour;

        static int LoadCustomIdsName(uint ids, wchar_t* buffer, int length);
        static int GetIdsNameOverride(void* resourceHandle, uint ids, wchar_t* buffer, int length);
        static char* __stdcall GetIdsInfocardOverride(uint ids);
        static void GetIdsInfocardNaked();
        static bool HandlePayload(const FlufPayload& payload);

    public:
        static void ClearOverrides();
        static void Initialise();
        static void UpdateInfocard(uint ids, const std::wstring& data, bool isName);

        struct InfocardPayload
        {
                std::unordered_map<uint, std::string> infoCards;
                std::unordered_map<uint, std::string> infoNames;
        };
};
