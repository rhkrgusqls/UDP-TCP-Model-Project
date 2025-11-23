#include "Session.h"

/** 생성자 */
Session::Session(int32 InSessionId)
    : SessionId(InSessionId)
    , SocketHandle(-1)
{
    // 세션 초기화 작업
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
    // 소켓을 통해 데이터 전송
}

/** 세션 종료 */
void Session::Close()
{
    // 소켓 닫기 및 관련 자원 정리
}
