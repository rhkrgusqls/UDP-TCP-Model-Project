#include "ClientUDPReceiver.h"
#include "UdpPacketHeader.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

ClientUDPReceiver::ClientUDPReceiver()
    : m_socket(-1)
{
}

ClientUDPReceiver::~ClientUDPReceiver()
{
    if (m_socket != -1)
        close(m_socket);
}

bool ClientUDPReceiver::Init(uint16_t port)
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0)
        return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(m_socket, (sockaddr*)&addr, sizeof(addr)) < 0)
        return false;

    return true;
}

void ClientUDPReceiver::Run()
{
    unsigned char buffer[1500];

    while (true)
    {
        int recvBytes = recvfrom(
            m_socket,
            buffer,
            sizeof(buffer),
            0,
            nullptr,
            nullptr
        );

        if (recvBytes <= 0)
            continue;

        // Forward raw packet to UDP model
        m_model.ProcessReceivedPacket(buffer, recvBytes);

        // Optional completion check
        // if (m_model.IsSessionComplete())
        // {
        //     m_model.MergeToFile("output.bin");
        // }
    }
}