#include "PCH.hpp"

#include "Internal/Hooks/InfocardOverrides.hpp"

#include "ClientServerCommunicator.hpp"
#include "Fluf.hpp"
#include "Utils/MemUtils.hpp"
#include "Utils/StringUtils.hpp"

int InfocardOverrides::GetIdsNameOverride(void* resourceHandle, uint ids, wchar_t* buffer, int length)
{
    if (const auto name = infoNameOverrides.find(ids); name != infoNameOverrides.end())
    {
        const auto size = name->second.size() * sizeof(wchar_t) + 2;
        memcpy_s(buffer, length, name->second.data(), size);
        return size > length ? size : length;
    }

    getIdsNameDetour->UnDetour();
    auto res = getIdsNameDetour->GetOriginalFunc()(resourceHandle, ids, buffer, length);
    getIdsNameDetour->Detour(GetIdsNameOverride);

    return res;
}

inline static uint infocardLength = 0;
inline static std::array<char, 65535> infocardBuffer;
char* __stdcall InfocardOverrides::GetIdsInfocardOverride(const uint ids)
{
    std::memset(infocardBuffer.data(), '\0', infocardBuffer.size());
    if (auto infocard = infoCardOverrides.find(ids); infocard != infoCardOverrides.end())
    {
        infocardLength = infocard->second.size() * 2;
        memcpy_s(infocardBuffer.data(), infocardBuffer.size(), infocard->second.data(), infocardLength);
        return infocardBuffer.data();
    }

    return nullptr;
}

constexpr DWORD NakedReturn = 0x57DB25;
__declspec(naked) void InfocardOverrides::GetIdsInfocardNaked()
{
    __asm {
        push eax // XMLReader VTable or something..?
        push [esp+0x18] // ids number
        call GetIdsInfocardOverride
        test eax, eax
        jz normalOperation

        mov edi, eax // Replace their buffer with our own
        pop eax // Restore old xml reader
        mov ebx, [esp+0x10+0x8] // RDL
        mov ecx, infocardLength
        jmp done

        normalOperation:
        pop eax // Restore old xml reader
        mov ebx, [esp+0x10+0x8] // RDL
        lea ecx, [esi+esi]

        done: // Default operation
        push ecx
        push edi
        push ebx
        jmp NakedReturn
    }
}

bool InfocardOverrides::HandlePayload(const FlufPayload& payload)
{
    if (payload.header != "infocard_override")
    {
        return false;
    }

    auto infoCardInfo = payload.Convert<InfocardPayload>();
    if (infoCardInfo.error())
    {
        Fluf::Error("Received infocard payload with invalid data");
        return false;
    }

    for (auto& [id, name] : infoCardInfo.value().infoNames)
    {
        infoNameOverrides[id] = StringUtils::stows(name);
    }

    for (auto& [id, card] : infoCardInfo.value().infoCards)
    {
        infoCardOverrides[id] = StringUtils::stows(card);
    }

    return true;
}

void InfocardOverrides::ClearOverrides()
{
    infoCardOverrides.clear();
    infoNameOverrides.clear();
}

void InfocardOverrides::Initialise()
{
    getIdsNameDetour = std::make_unique<FunctionDetour<GetIdsName>>(reinterpret_cast<GetIdsName>(0x4347e0));
    getIdsNameDetour->Detour(GetIdsNameOverride);

    MemUtils::PatchAssembly(0x57DB1B, GetIdsInfocardNaked);
}

void InfocardOverrides::UpdateInfocard(const uint ids, const std::wstring& data, const bool isName)
{
    if (isName)
    {
        infoNameOverrides[ids] = data;
    }
    else
    {
        infoCardOverrides[ids] = data;
    }
}
