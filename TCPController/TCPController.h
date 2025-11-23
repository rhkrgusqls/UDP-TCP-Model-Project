#pragma once
#include "ITCPController.h"
#include "Session.h"
#include <unordered_map>

/** TCP 연결을 관리하는 컨트롤러 클래스
    세션 생성, 메시지 전송, 이벤트 수신 등을 담당
*/
class TCPController : public ITCPController
{
public:
    /** 생성자
        내부 상태 초기화
    */
    TCPController();

    /** 소멸자
        Shutdown() 호출하여 모든 세션 정리
    */
    virtual ~TCPController();

    /** TCP 컨트롤러 초기화
        TCP 시스템 준비 작업 수행
        @return 성공 시 true, 실패 시 false
    */
    virtual bool Init() override;

    /** TCP 컨트롤러 종료
        모든 세션 종료 및 자원 정리
    */
    virtual void Shutdown() override;

    /** 새로운 사용자 세션 생성
        @input ConnectionId 세션 고유 아이디
        @return 생성된 세션 객체
    */
    virtual ISession* CreateSession(int32 ConnectionId) override;

    /** 특정 세션에 메시지 전송
        @input SessionId 대상 세션 아이디
        @input Message 전송할 메시지
    */
    virtual void SendToSession(int32 SessionId, const std::string& Message) override;

    /** 모든 세션에 메시지 브로드캐스트
        @input Message 전송할 메시지
    */
    virtual void Broadcast(const std::string& Message) override;

    /** 외부 모델에서 TCP로 옵저버 이벤트 전달
        @input EventName 이벤트 이름
        @input Payload 이벤트 데이터
    */
    virtual void OnNotifyEvent(const std::string& EventName, const std::string& Payload) override;

private:
    /** 현재 활성화된 모든 세션을 저장하는 컨테이너
        key: 세션 ID
        value: 세션 객체
    */
    std::unordered_map<int32, ISession*> Sessions;
};
