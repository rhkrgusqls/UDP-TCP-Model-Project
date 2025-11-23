#include "UDPModel.h"
#include <iostream>
#include <cstring> // memcpy 등

UDPModel::UDPModel() : m_sessionId(0), m_totalPackets(0) {
    // 생성자 초기화
}

UDPModel::~UDPModel() {
    // 소멸자 (필요 시 자원 해제)
}

int UDPModel::InitializeSession(uint64_t sessionId, uint64_t totalPackets, const char* filename) {
    std::lock_guard<std::mutex> lock(m_mutex); // 스레드 안전하게 잠금

    m_sessionId = sessionId;
    m_totalPackets = totalPackets;
    m_oUDPutFilename = filename;

    // 버퍼 크기 잡기 (예시)
    m_packetBuffer.resize(totalPackets);
    m_receivedStatus.resize(totalPackets, false);

    std::cout << "[Model] Session Initialized. ID: " << m_sessionId << std::endl;
    return 1;
}

int UDPModel::ProcessReceivedPacket(const unsigned char* rawData, int length) {
    if (length < sizeof(UdpPacketHeader)) {
        return -1; // 헤더보다 작으면 에러
    }

    // 1. 헤더 파싱
    const UdpPacketHeader* header = reinterpret_cast<const UdpPacketHeader*>(rawData);

    // 2. 세션 확인
    if (header->session_id != m_sessionId) {
        return -1; // 내 세션 패킷이 아닐
    }

    // 3. 데이터 저장 (Critical Section)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 인덱스 범위 체크
        if (header->packet_index >= m_totalPackets) return -1;

        // 데이터 복사
        const unsigned char* payload = rawData + sizeof(UdpPacketHeader);
        m_packetBuffer[header->packet_index].assign(payload, payload + header->data_length);
        m_receivedStatus[header->packet_index] = true;
    }

    // 4. 옵저버 패턴: 외부로 알림 (TCP 핸들러 등이 받음)
    if (m_callback) {
        m_callback(header->session_id, header->packet_index, 1);
    }

    return 1;
}

int UDPModel::SendData(const unsigned char* data, int length) {
    // 여기에 sendto() 등을 이용한 UDP 전송 로직 구현
    // 현재는 모델 구조만 잡는 것이므로 성공(1) 리턴
    return 1; 
}

void UDPModel::SetStatusCallback(UdpPacketCallback callback) {
    m_callback = callback;
}