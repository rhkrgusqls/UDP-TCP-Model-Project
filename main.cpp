// main.cpp
#include <iostream>
#include "UTPModel.h"

int main() {
    // 1. 객체 생성 (인터페이스 포인터로 받기)
    IUTPModel* model = new UTPModel();

    // 2. 콜백 등록 (람다 함수 사용)
    model->SetStatusCallback([](uint64_t sessId, uint64_t pktIdx, int status) {
        std::cout << "[Callback] Session: " << sessId 
                  << ", Packet: " << pktIdx 
                  << " -> Status: " << status << std::endl;
    });

    // 3. 세션 초기화 테스트
    model->InitializeSession(100, 5, "test.txt");

    // 4. 가짜 데이터 수신 테스트 (패킷 헤더 + 데이터) 
    // 헤더 구조: [SessionID(8)][PacketIdx(8)][DataLen(4)] + [Data...]
    struct {
        uint64_t sid = 100;
        uint64_t pidx = 0;
        uint32_t len = 5;
        char data[5] = {'H', 'e', 'l', 'l', 'o'};
    } dummyPacket;

    // 강제로 char* 로 캐스팅해서 넣어봄
    model->ProcessReceivedPacket((unsigned char*)&dummyPacket, sizeof(dummyPacket));

    delete model;
    return 0;
}