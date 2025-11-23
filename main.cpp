#include "FileSplitterAndMerger.h"
#include <iostream>
#include <fstream>

/**
 * @brief FileSplitterAndMerger 모듈 단독 테스트용 main 함수
 *
 * 1. input.bin 이라는 테스트 파일을 만든다. (A~Z 패턴으로 10000바이트)
 * 2. FileSplitterAndMerger::SplitFile 로 파일을 1024바이트 단위 Packet으로 분할
 * 3. 각 Packet을 EncodePacket -> 문자열로 변환했다가
 *    다시 DecodePacket -> Packet으로 복원 (네트워크 전송 과정 흉내)
 * 4. 복원된 Packet 벡터를 MergeFile 로 output.bin 으로 병합
 * 5. input.bin 과 output.bin 내용을 비교해서 완전히 같은지 확인
 */
int main() {
    // ============================================================
    // 1) 테스트용 input.bin 파일 생성
    //    - 실제로는 사용자가 선택한 파일이 될 부분
    // ============================================================
    {
        std::ofstream f("input.bin", std::ios::binary);
        if (!f) {
            std::cout << "failed to create input.bin\n";
            return 1;
        }

        // 0 ~ 9999 까지 반복하면서 'A' ~ 'Z' 문자 패턴으로 기록
        // i % 26 을 이용해 0~25 사이 숫자로 만들고, 'A'를 더해서 알파벳으로 변환
        for (int i = 0; i < 10000; ++i) {
            char c = static_cast<char>('A' + (i % 26)); // A B C ... Z 반복
            f.put(c);
        }
    }

    FileSplitterAndMerger fsm; // 파일 분할/병합 모듈 객체

    // ============================================================
    // 2) input.bin 파일을 1024바이트 단위로 분할
    // ============================================================
    auto packets = fsm.SplitFile("input.bin", 1024);
    std::cout << "packets: " << packets.size() << "\n";

    // ============================================================
    // 3) EncodePacket -> 문자열, DecodePacket -> Packet
    //    네트워크에서 문자열을 주고받는 과정을 흉내낸다.
    // ============================================================
    std::vector<Packet> received;
    received.reserve(packets.size());

    for (const auto& p : packets) {
        // Packet -> 문자열
        std::string raw = FileSplitterAndMerger::EncodePacket(p);

        // 문자열 -> Packet
        Packet q;
        bool ok = FileSplitterAndMerger::DecodePacket(raw, q);
        if (!ok) {
            std::cout << "decode failed\n";
            return 1;
        }

        received.push_back(std::move(q));
    }

    // ============================================================
    // 4) 수신된 Packet 목록을 output.bin 으로 병합
    // ============================================================
    bool ok = fsm.MergeFile("output.bin", received);
    std::cout << "merge: " << ok << "\n";

    // ============================================================
    // 5) input.bin 과 output.bin 의 내용이 완전히 같은지 비교
    //    - equal: 1 이면 "성공적으로 분할+병합이 잘 동작했다"는 뜻
    // ============================================================
    std::ifstream a("input.bin", std::ios::binary);
    std::ifstream b("output.bin", std::ios::binary);
    if (!a || !b) {
        std::cout << "failed to open input.bin or output.bin\n";
        return 1;
    }

    std::string sa((std::istreambuf_iterator<char>(a)), {});
    std::string sb((std::istreambuf_iterator<char>(b)), {});

    std::cout << "equal: " << (sa == sb) << "\n";

    return 0;
}
