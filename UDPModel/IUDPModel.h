#ifndef I_UDP_MODEL_H
#define I_UDP_MODEL_H

#include <cstdint> // uint64_t, uint32_t 등 사용
#include <functional> // 콜백 함수(std::function) 사용

// 옵저버 패턴: 패킷 처리 결과를 알려주는 콜백 함수 타입 정의
// (세션ID, 패킷번호, 상태코드)를 인자로 받음
using UdpPacketCallback = std::function<void(uint64_t, uint64_t, int)>;

class IUDPModel {
public:
    virtual ~IUDPModel() = default;

    /**
     * @brief 전송 세션을 초기화합니다. (TCP로 메타데이터 교환 후 호출)
     * @param sessionId 고유 세션 ID
     * @param totalPackets 전체 패킷 개수
     * @param filename 저장할 파일 이름
     * @return 성공 시 1, 실패 시 -1
     */
    virtual int InitializeSession(uint64_t sessionId, uint64_t totalPackets, const char* filename) = 0;

    /**
     * @brief UDP로 수신된 로우(Raw) 데이터를 처리합니다.
     * 내부에서 패킷 헤더를 분석하고 데이터를 버퍼에 저장한 뒤, 콜백을 호출합니다.
     * * @param rawData 수신된 바이너리 데이터 포인터
     * @param length 데이터 길이
     * @return 처리 성공 시 1, 실패(잘못된 패킷 등) 시 -1
     */
    virtual int ProcessReceivedPacket(const unsigned char* rawData, int length) = 0;

    /**
     * @brief 데이터 전송 함수 (Sender 역할일 경우 사용)
     * 데이터를 패킷 단위로 쪼개서 전송합니다.
     * * @param data 보낼 데이터 포인터
     * @param length 데이터 전체 길이
     * @return 전송 성공 시 1, 실패 시 -1
     */
    virtual int SendData(const unsigned char* data, int length) = 0;

    /**
     * @brief 패킷 처리 결과를 받을 옵저버(리스너)를 등록합니다.
     * @param callback 호출될 함수
     */
    virtual void SetStatusCallback(UdpPacketCallback callback) = 0;
};

#endif // I_UDP_MODEL_H