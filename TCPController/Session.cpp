#include "Session.h"
#include <unistd.h>
#include <sys/socket.h>
/** 생성자 */
Session::Session(int32 InSessionId)
    : SessionId(InSessionId)
    , SocketHandle(-1)
{
}
/** 소멸자 */
Session::~Session()
{
    Close();
}
/** 세션 ID 반환 */
int32 Session::GetId() const
{
    return SessionId;
}
/** 메시지 전송 */
void Session::Send(const std::string& Message)
{
    if (SocketHandle == -1)
        return;

    // Send message to client socket
    send(SocketHandle, Message.c_str(), Message.size(), 0);
}
/** 세션 종료 */
void Session::Close()
{
    if (SocketHandle != -1)
    {
        // Close TCP socket
        close(SocketHandle);
        SocketHandle = -1;
    }
}
