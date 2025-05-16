#include "PCH.hpp"

#include "ClientServerCommunicator.hpp"

#include "FlufModule.hpp"
#include "Internal/Hooks/InfocardOverrides.hpp"
#include "Utils/MemUtils.hpp"

void ClientServerCommunicator::OnReceiveChatMessageClient(uint sourceClientId, const uint destClientId, const size_t size, char* data)
{
    if (size < 8 || *reinterpret_cast<USHORT*>(data) != flufHeader)
    {
        oldSendChat(sourceClientId, destClientId, size, data);
        return;
    }

    char* dataOffset = data + sizeof(flufHeader);

    std::array<char, 4> header{};
    memcpy_s(header.data(), header.size(), dataOffset, 4);

    dataOffset += header.size();
    const size_t newDataSize = size - sizeof(flufHeader) - header.size();

    if (InfocardOverrides::HandlePayload(header, dataOffset, newDataSize))
    {
        return;
    }

    if (Fluf::instance->CallModuleEvent(&FlufModule::OnPayloadReceived, sourceClientId, header, dataOffset, newDataSize))
    {
        oldSendChat(sourceClientId, destClientId, size, data);
    }
}

void ClientServerCommunicator::OnReceiveChatMessageServer(uint sourceClientId, size_t size, char* data, uint destTargetId, int unknown)
{
    if (size < 8 || *reinterpret_cast<unsigned long*>(data) != flufHeader)
    {
        oldSubmitChat(sourceClientId, size, data, destTargetId, unknown);
        return;
    }

    char* dataOffset = data + sizeof(flufHeader);

    std::array<char, 4> header{};
    memcpy_s(header.data(), header.size(), data + sizeof(flufHeader), sizeof(flufHeader));

    dataOffset += header.size();
    const size_t newDataSize = size - sizeof(flufHeader) - header.size();

    if (Fluf::instance->CallModuleEvent(&FlufModule::OnPayloadReceived, sourceClientId, header, dataOffset, newDataSize))
    {
        oldSubmitChat(sourceClientId, size, data, destTargetId, unknown);
    }
}

ClientServerCommunicator::ClientServerCommunicator()
{
    if (Fluf::IsRunningOnClient())
    {
        MemUtils::ReadProcMem(clientReceivePayloadAddress, &oldSendChat, sizeof oldSendChat);
        const auto proc = reinterpret_cast<FARPROC>(&ClientServerCommunicator::OnReceiveChatMessageClient);
        MemUtils::WriteProcMem(clientReceivePayloadAddress, proc, sizeof(proc));
    }
    else
    {
        const auto server = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA("server.dll"), "Server"));
        const auto submitChat = server - 0x08;

        MemUtils::ReadProcMem(submitChat, &oldSubmitChat, sizeof(oldSubmitChat));
        const auto newSubmitChat = &ClientServerCommunicator::OnReceiveChatMessageServer;
        MemUtils::WriteProcMem(submitChat, &newSubmitChat, sizeof(newSubmitChat));
    }
}

ClientServerCommunicator::~ClientServerCommunicator() = default;
