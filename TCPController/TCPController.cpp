#include "TCPController.h"

// ------------------------------------
// 생성자 / 소멸자
// ------------------------------------

TCPController::TCPController()
{
    /** 내부 상태 초기화 */
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
    /** TCP 소켓 시스템 초기화
        @return 성공 시 true, 실패 시 false
    */
    return true;
}

void TCPController::Shutdown()
{
    /** 모든 세션 종료 및 자원 정리 */
    for (auto& Pair : Sessions)
    {
        Pair.second->Close();
        delete Pair.second;
    }
    Sessions.clear();
}

// ------------------------------------
// Session Management
// ------------------------------------

ISession* TCPController::CreateSession(int32 ConnectionId)
{
    /** 새로운 세션 생성
        @input ConnectionId 세션 고유 아이디
        @return 생성된 세션 객체
    */
    ISession* NewSession = new Session(ConnectionId);
    Sessions.emplace(ConnectionId, NewSession);
    return NewSession;
}

void TCPController::SendToSession(int32 SessionId, const std::string& Message)
{
    /** 특정 세션에 메시지 전송
        @input SessionId 대상 세션 아이디
        @input Message 전송할 데이터
    */
    if (Sessions.contains(SessionId))
        Sessions[SessionId]->Send(Message);
}

void TCPController::Broadcast(const std::string& Message)
{
    /** 모든 세션에 메시지 브로드캐스트
        @input Message 전송할 데이터
    */
    for (auto& Pair : Sessions)
        Pair.second->Send(Message);
}

// ------------------------------------
// Observer Event Receiver
// ------------------------------------

void TCPController::OnNotifyEvent(const std::string& EventName, const std::string& Payload)
{
    /** 외부 모델에서 전달된 이벤트 처리
        @input EventName 이벤트 이름
        @input Payload 이벤트 데이터
        이벤트를 적절히 조합하여 전체 세션에 전달
    */
    std::string Combined = EventName + ":" + Payload;
    Broadcast(Combined);
}
