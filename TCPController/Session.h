#pragma once
#include <string>

/** TCP 통신에서 단일 유저 연결을 표현하는 실제 세션 클래스
*/
class Session
{
public:
    /** 생성자
        @input InSessionId 세션 고유 아이디
    */
    Session(int32 InSessionId);

    /** 소멸자
        세션 종료 처리 포함
    */
    ~Session();

    /** 세션 고유 아이디 반환
        @return 세션 ID
    */
    int32 GetId() const;

    /** 클라이언트로 메시지 전송
        @input Message 전송할 메시지
    */
    void Send(const std::string& Message);

    /** 세션 종료 함수
        소켓 종료, 버퍼 정리 등 수행
    */
    void Close();

private:
    /** 세션 고유 아이디 */
    int32 SessionId;

    /** 내부 소켓 핸들 혹은 네트워크 연결 객체
        실제 구현은 TCPController.cpp 또는 네트워크 모듈에서 처리
    */
    int32 SocketHandle;
};
