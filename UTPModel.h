#ifndef UTP_MODEL_H
#define UTP_MODEL_H

#include "IUTPModel.h"
#include <vector>
#include <string>
#include <mutex>

#pragma pack(push, 1)
struct UdpPacketHeader {
    uint64_t session_id;
    uint64_t packet_index;
    uint32_t data_length;
};
#pragma pack(pop)

class UTPModel : public IUTPModel {
private:
    // 멤버 변수들 (private으로 숨김)
    uint64_t m_sessionId;
    uint64_t m_totalPackets;
    std::string m_outputFilename;
    
    // 데이터 버퍼 (vector 사용 권장)
    std::vector<std::vector<unsigned char>> m_packetBuffer;
    std::vector<bool> m_receivedStatus;
    
    // 동기화를 위한 뮤텍스
    std::mutex m_mutex;

    // 콜백 함수 저장소
    UdpPacketCallback m_callback;

public:
    UTPModel();
    virtual ~UTPModel();

    // 인터페이스 구현부 (override 키워드로 명시)
    int InitializeSession(uint64_t sessionId, uint64_t totalPackets, const char* filename) override;
    int ProcessReceivedPacket(const unsigned char* rawData, int length) override;
    int SendData(const unsigned char* data, int length) override;
    void SetStatusCallback(UdpPacketCallback callback) override;
};

#endif 