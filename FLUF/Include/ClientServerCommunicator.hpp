#pragma once

#include "FLCore/FLCoreServer.h"
#include "Fluf.hpp"

#include <rfl/msgpack.hpp>

struct IChatServer
{
        virtual void SubmitChat(uint sourceClientId, size_t payloadSize, void* buffer, uint clientId, uint unknown);
};

class Fluf;
class InfocardOverrides;
class ClientServerCommunicator
{
        using ServerSendChat = void(__stdcall*)(uint sourceId, uint targetId, size_t size, const char* data);
        using ClientSendChat = void(__stdcall*)(uint sourceClientId, size_t size, const char* data, uint destTargetId, int unknown);

        friend Fluf;
        static constexpr DWORD clientReceivePayloadAddress = 0x5E6668;
        static constexpr USHORT flufHeader = 0xF10F;
        inline static ClientSendChat oldSubmitChat;
        inline static ServerSendChat oldSendChat;

        IChatServer* clientChatServer = nullptr;

    public:
        static void __stdcall OnReceiveChatMessageClient(uint sourceClientId, uint destClientId, size_t size, char* data);
        static void __stdcall OnReceiveChatMessageServer(uint sourceClientId, size_t size, char* data, uint destTargetId, int unknown);
        template <typename T>
        bool SendPayloadFromClient(const uint targetId, const std::array<char, 4>& header, const T& payload)
        {
            if (!clientChatServer)
            {
                return false;
            }

            auto msgPack = rfl::msgpack::write<T>(payload);
            const size_t size = msgPack.size() + header.size() + 4;
            std::vector<char> data(size);

            memcpy_s(data.data(), data.size(), &flufHeader, sizeof(flufHeader));
            memcpy_s(data.data() + sizeof(flufHeader), data.size(), header.data(), header.size());
            memcpy_s(data.data() + msgPack.size() + sizeof(flufHeader), data.size() - msgPack.size(), msgPack.data(), msgPack.size());
            clientChatServer->SubmitChat(*reinterpret_cast<uint*>(0x673344), data.size(), data.data(), targetId, -1);

            return true;
        }

        template <typename T>
        bool SendPayloadFromServer(const uint targetId, const std::array<char, 4>& header, const T& payload)
        {
            if (Fluf::IsRunningOnClient())
            {
                return false;
            }

            auto msgPack = rfl::msgpack::write<T>(payload);
            const size_t size = msgPack.size() + header.size() + sizeof(flufHeader);
            std::vector<char> data(size);

            memcpy_s(data.data(), data.size(), &flufHeader, sizeof(flufHeader));
            memcpy_s(data.data() + sizeof(flufHeader), data.size(), header.data(), header.size());
            memcpy_s(data.data() + msgPack.size() + sizeof(flufHeader), data.size() - msgPack.size(), msgPack.data(), msgPack.size());

            static auto sendChat = reinterpret_cast<ServerSendChat>(reinterpret_cast<DWORD>(GetModuleHandleA("remoteclient.dll")) + 0x3BB80);
            sendChat(*reinterpret_cast<uint*>(0x673344), targetId, data.size(), data.data());

            return true;
        }

        ClientServerCommunicator();
        ~ClientServerCommunicator();
};
