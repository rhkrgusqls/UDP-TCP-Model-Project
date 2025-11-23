#pragma once

class ISession;

class ITCPController
{
public:
    virtual ~ITCPController() {}

    /** TCP 시스템 초기화 함수
        최초 생성 시 반드시 호출 필요
        @return 성공 시 true, 실패 시 false
    */
    virtual bool Init() = 0;

    /** TCP 서버를 안정적으로 종료시키는 함수
        내부 세션과 자원을 정리함
    */
    virtual void Shutdown() = 0;

    /** 새로운 세션 생성 함수
        @input ConnectionId 연결 고유 아이디
        @return 유저와의 연결(Session) 객체 반환
    */
    virtual ISession* CreateSession(int32 ConnectionId) = 0;

    /** 특정 세션에 메시지 전송
        @input SessionId 대상 세션 아이디
        @input Message 전송할 메시지
    */
    virtual void SendToSession(int32 SessionId, const std::string& Message) = 0;

    /** 전체 세션에 메시지 전송 (브로드캐스트)
        @input Message 전송할 메시지
    */
    virtual void Broadcast(const std::string& Message) = 0;

    /** 외부 모델에서 TCP로 이벤트 전달
        @input EventName 이벤트 이름
        @input Payload 이벤트 데이터
    */
    virtual void OnNotifyEvent(const std::string& EventName, const std::string& Payload) = 0;
};
