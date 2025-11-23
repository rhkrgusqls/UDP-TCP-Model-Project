#include "FileSplitterAndMerger.h"

#include <fstream>   // ifstream, ofstream (파일 입출력)
#include <algorithm> // std::sort
#include <iostream>  // std::cerr, std::cout
#include <sstream>   // std::ostringstream

// ================================================================
//  SplitFile 구현: 파일 -> Packet 벡터
// ================================================================
std::vector<Packet> FileSplitterAndMerger::SplitFile(const std::string& filePath,
                                                     std::size_t payloadSize)
{
    std::vector<Packet> packets; // 결과를 담을 벡터

    // 1) payloadSize 유효성 체크
    if (payloadSize == 0) {
        // 0으로 잘라야 한다는 건 말이 안 되기 때문에 바로 리턴
        std::cerr << "[SplitFile] payloadSize must be > 0\n";
        return packets;
    }

    // 2) 파일 열기 (바이너리 모드)
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        // 파일이 없거나 권한 문제 등으로 열지 못한 경우
        std::cerr << "[SplitFile] Failed to open file: " << filePath << "\n";
        return packets;
    }

    // 3) payloadSize 크기만큼 읽어올 임시 버퍼
    std::vector<char> buffer(payloadSize);
    uint32_t seq = 0; // 패킷 번호 (0부터 시작)

    // 4) 파일 끝까지 반복해서 읽으면서 Packet 생성
    while (in) {
        // 최대 payloadSize 바이트까지 읽기
        in.read(buffer.data(), static_cast<std::streamsize>(payloadSize));
        std::streamsize bytesRead = in.gcount(); // 실제로 읽힌 바이트 수

        if (bytesRead <= 0) {
            // 더 이상 읽을 데이터가 없으면 루프 종료
            break;
        }

        // 5) Packet 하나 생성
        Packet p;
        p.seq    = seq++; // 현재 패킷 번호 설정 후 다음 패킷을 위해 증가
        p.length = static_cast<uint32_t>(bytesRead);

        // buffer 의 앞부분 bytesRead 만큼만 data로 복사
        p.data.assign(buffer.data(),
                      buffer.data() + static_cast<std::size_t>(bytesRead));

        // 6) 결과 벡터에 추가
        packets.push_back(std::move(p));
    }

    // 7) 모든 패킷 생성 완료 후 반환
    return packets;
}

// ================================================================
//  MergeFile 구현: Packet 벡터 -> 파일
// ================================================================
bool FileSplitterAndMerger::MergeFile(const std::string& outFilePath,
                                      const std::vector<Packet>& packets)
{
    // 1) 입력 패킷이 비어 있으면 병합할 게 없음
    if (packets.empty()) {
        std::cerr << "[MergeFile] packets is empty\n";
        return false;
    }

    // 2) seq 기준으로 정렬 (네트워크에서 순서가 섞여 들어올 수 있기 때문)
    std::vector<Packet> ordered = packets;
    std::sort(ordered.begin(), ordered.end(),
              [](const Packet& a, const Packet& b) {
                  return a.seq < b.seq;
              });

    // 3) 출력 파일 열기 (바이너리 모드, 기존 내용 삭제)
    std::ofstream out(outFilePath,
                      std::ios::binary | std::ios::trunc);
    if (!out) {
        std::cerr << "[MergeFile] Failed to open output file: "
                  << outFilePath << "\n";
        return false;
    }

    // 4) 정렬된 순서대로 data를 이어붙여 파일에 기록
    for (const auto& p : ordered) {
        // length가 data 크기보다 크면 잘못된 패킷
        if (p.length > p.data.size()) {
            std::cerr << "[MergeFile] Invalid packet length (seq=" << p.seq << ")\n";
            return false;
        }

        // data의 앞부분 length만큼 파일에 쓰기
        out.write(p.data.data(),
                  static_cast<std::streamsize>(p.length));
    }

    // 5) 파일 쓰기 상태 확인 후 반환
    return static_cast<bool>(out);
}

// ================================================================
//  Packet -> 문자열 인코딩
//  포맷: [seq]|[length]{data}
//  예:   0|5{hello}
// ================================================================
std::string FileSplitterAndMerger::EncodePacket(const Packet& p)
{
    // 문자열을 효율적으로 만들기 위해 stringstream 사용
    std::ostringstream oss;

    // 1) seq와 length를 "[seq]|[length]" 형식으로 출력
    oss << p.seq
        << PACKET_DELIM     // '|' 문자
        << p.length
        << PACKET_START;    // '{' 문자

    // 2) data 내용을 그대로 기록 (바이너리/텍스트 모두 가능)
    oss.write(p.data.data(),
              static_cast<std::streamsize>(p.length));

    // 3) data 끝을 표시하는 '}' 추가
    oss << PACKET_END;

    // 4) 완성된 문자열 반환
    return oss.str();
}

// ================================================================
//  문자열 -> Packet 디코딩
//  포맷: [seq]|[length]{data}
//  문자열에서 seq, length, data를 분리해 Packet으로 만든다.
// ================================================================
bool FileSplitterAndMerger::DecodePacket(const std::string& raw, Packet& out)
{
    // 1) seq와 length를 구분하는 '|' 위치 찾기
    auto delimPos = raw.find(PACKET_DELIM);
    if (delimPos == std::string::npos) {
        std::cerr << "[DecodePacket] DELIM not found\n";
        return false;
    }

    // 2) length와 data를 구분하는 '{' 위치 찾기
    auto startPos = raw.find(PACKET_START, delimPos + 1);
    if (startPos == std::string::npos) {
        std::cerr << "[DecodePacket] START not found\n";
        return false;
    }

    // 3) 0 ~ delimPos-1 : seq 문자열
    //    delimPos+1 ~ startPos-1 : length 문자열
    std::string seqStr = raw.substr(0, delimPos);
    std::string lenStr = raw.substr(delimPos + 1,
                                    startPos - (delimPos + 1));

    uint32_t seq = 0;
    uint32_t len = 0;

    // 4) 문자열을 정수형으로 변환 (예: "10" -> 10)
    try {
        seq = static_cast<uint32_t>(std::stoul(seqStr));
        len = static_cast<uint32_t>(std::stoul(lenStr));
    } catch (const std::exception& e) {
        std::cerr << "[DecodePacket] stoi error: " << e.what() << "\n";
        return false;
    }

    // 5) data 시작 위치 계산 ( '{' 바로 다음 )
    std::size_t dataStart = startPos + 1;

    // raw 길이가 dataStart + len 보다 작으면 데이터가 부족한 것
    if (raw.size() < dataStart + len) {
        std::cerr << "[DecodePacket] raw too short for data\n";
        return false;
    }

    // 6) data 뒤에 '}'가 제대로 있는지 확인
    // data는 len 바이트이므로, 그 다음 위치에 '}'가 와야 한다고 가정
    std::size_t endPos = dataStart + len;
    if (endPos >= raw.size() || raw[endPos] != PACKET_END) {
        std::cerr << "[DecodePacket] END not found at expected position\n";
        return false;
    }

    // 7) 실제 데이터 부분만 잘라서 저장
    std::string data = raw.substr(dataStart, len);

    // 8) out Packet에 결과 채우기
    out.seq    = seq;
    out.length = len;
    out.data   = std::move(data);

    return true;
}
