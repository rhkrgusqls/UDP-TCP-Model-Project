#include "TCPController.h"
#include "Session.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

// ------------------------------------
// 생성자 / 소멸자
// ------------------------------------

TCPController::TCPController()
    : ListenSocket(-1)
{
    /** Internal state initialization */
}

TCPController::~TCPController()
{
    Shutdown();
}

// ------------------------------------
// TCP Init / Shutdown
// ------------------------------------

bool TCPController::Init()
{
    /** Initialize TCP socket system */

    ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ListenSocket < 0)
        return false;

    sockaddr_in Addr{};
    Addr.sin_family = AF_INET;
    Addr.sin_addr.s_addr = INADDR_ANY;
    Addr.sin_port = htons(7777);

    if (bind(ListenSocket, (sockaddr*)&Addr, sizeof(Addr)) < 0)
        return false;

    if (listen(ListenSocket, SOMAXCONN) < 0)
        return false;

    /** Initialize UDP socket for file transfer */

    UdpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (UdpSocket < 0)
        return false;

    return true;
}


void TCPController::Shutdown()
{
    for (auto& Pair : Sessions)
    {
        Pair.second->Close();
        delete Pair.second;
    }
    Sessions.clear();

    if (ListenSocket != -1)
    {
        close(ListenSocket);
        ListenSocket = -1;
    }

    if (UdpSocket != -1)
    {
        close(UdpSocket);
        UdpSocket = -1;
    }
}

// ------------------------------------
// Session Management
// ------------------------------------

ISession* TCPController::CreateSession(int32 ConnectionId)
{
    /** Create new session */

    ISession* NewSession = new Session(ConnectionId);
    Sessions.emplace(ConnectionId, NewSession);
    return NewSession;
}

void TCPController::SendToSession(int32 SessionId, const std::string& Message)
{
    /** Send message to specific session */

    if (Sessions.contains(SessionId))
        Sessions[SessionId]->Send(Message);
}

void TCPController::Broadcast(const std::string& Message)
{
    /** Broadcast message to all sessions */

    for (auto& Pair : Sessions)
        Pair.second->Send(Message);
}

// ------------------------------------
// Network Accept / Update
// ------------------------------------

void TCPController::AcceptClient()
{
    /** Accept incoming TCP connection */

    int ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket < 0)
        return;

    int32 SessionId = ClientSocket;

    // Create session object
    ISession* NewSession = CreateSession(SessionId);

    // Assign socket handle to session (internal access)
    static_cast<Session*>(NewSession)->SocketHandle = ClientSocket;
}

void TCPController::Update()
{
    /** Iterate sessions and process incoming commands */

    char Buffer[1024];

    for (auto& Pair : Sessions)
    {
        Session* SessionObj = static_cast<Session*>(Pair.second);
        int Socket = SessionObj->SocketHandle;

        memset(Buffer, 0, sizeof(Buffer));

        int RecvBytes = recv(Socket, Buffer, sizeof(Buffer), MSG_DONTWAIT);
        if (RecvBytes <= 0)
            continue;

        std::string Command(Buffer, RecvBytes);

        // Process command per session
        ProcessCommand(SessionObj, Command);
    }
}

// ------------------------------------
// Command Routing
// ------------------------------------

void TCPController::ProcessCommand(Session* SessionObj, const std::string& Command)
{
    /** Route behavior based on command */

    if (Command.starts_with("FILE_SEND "))
    {
        // FILE_SEND <filename> <client_ip> <udp_port>

        std::istringstream iss(Command);
        std::string cmd, filename, ip;
        int port;

        iss >> cmd >> filename >> ip >> port;

        /** Setup client UDP address */
        ClientUdpAddr.sin_family = AF_INET;
        ClientUdpAddr.sin_port = htons(port);
        ClientUdpAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        /** Split file */
        FileSplitterAndMerger fsm;
        auto packets = fsm.SplitFile(filename, 1024);

        uint64_t sessionId = SessionObj->GetId();
        uint64_t totalPackets = packets.size();

        /** Cache packets for retransmission */
        SentPacketCache[sessionId] = packets;

        for (uint64_t i = 0; i < packets.size(); ++i)
        {
            SendUdpPacket(sessionId, i, packets[i], totalPackets);
            usleep(1000);
        }

        /** Notify client */
        SessionObj->Send("FILE_SEND_DONE");
    }
    else if (Command.starts_with("FILE_RESEND "))
    {
        // FILE_RESEND <packet_index>

        std::istringstream iss(Command);
        std::string cmd;
        uint64_t packetIndex;

        iss >> cmd >> packetIndex;

        uint64_t sessionId = SessionObj->GetId();

        if (!SentPacketCache.contains(sessionId))
            return;

        auto& packets = SentPacketCache[sessionId];

        if (packetIndex >= packets.size())
            return;

        /** Resend missing packet */
        SendUdpPacket(
            sessionId,
            packetIndex,
            packets[packetIndex],
            packets.size()
        );
    }
}



// ------------------------------------
// Observer Event Receiver
// ------------------------------------

void TCPController::OnNotifyEvent(const std::string& EventName, const std::string& Payload)
{
    /** Handle external event notification */

    std::string Combined = EventName + ":" + Payload;
    Broadcast(Combined);
}
