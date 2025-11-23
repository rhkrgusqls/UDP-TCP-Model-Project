#ifndef IFILE_SPLITTER_AND_MERGER_H
#define IFILE_SPLITTER_AND_MERGER_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief 한 개의 "전송 단위"를 나타내는 구조체
 *
 * - 파일을 그대로 보내지 않고, 일정 크기(payloadSize)로 잘라서
 *   네트워크로 보내기 위해 사용하는 최소 단위가 Packet이다.
 * - seq, length, data 를 가지며, 수신 쪽에서는 이 정보를 이용해
 *   원래 파일 순서대로 다시 조립(병합)한다.
 */
struct Packet {
    uint32_t seq;        // 패킷 번호 (0부터 시작하는 순번, 0, 1, 2, ...)
    uint32_t length;     // data에 실제로 들어있는 바이트 수
    std::string data;    // 실제 데이터 (이 패킷에 담긴 내용, 바이너리/텍스트 모두 가능)
};

/**
 * @brief 파일을 Packet 단위로 "분할"하고, 여러 Packet을 다시 "병합"하는 기능에 대한 인터페이스
 *
 * - 이 인터페이스를 상속받는 클래스(예: FileSplitterAndMerger)가
 *   실제 구현을 담당한다.
 * - 상위 모듈(UTPModel 등)은 이 인터페이스만 알고,
 *   실제 구현 클래스에 의존하지 않고 사용할 수 있게 하기 위한 구조이다.
 */
class IFileSplitterAndMerger {
public:
    /**
     * @brief 가상 소멸자
     *
     * - 인터페이스(추상 클래스)를 포인터로 다룰 때,
     *   자식 클래스의 소멸자가 제대로 호출되도록 하기 위해
     *   virtual 로 선언한다.
     * - = default 는 "기본 구현을 사용한다"는 의미.
     */
    virtual ~IFileSplitterAndMerger() = default;

    /**
     * @brief 파일을 읽어 payloadSize 크기만큼 Packet 벡터로 분할한다.
     *
     * @param filePath
     *   분할할 원본 파일 경로 (예: "input.bin", "/tmp/data.bin")
     *
     * @param payloadSize
     *   한 개의 Packet에 담을 최대 바이트 수
     *   - payloadSize가 1024라면,
     *     파일을 최대 1024바이트씩 잘라 여러 Packet으로 만든다.
     *
     * @return
     *   - 성공 시: 분할된 Packet들을 순서대로 담은 벡터
     *   - 실패 시: 빈 벡터 (파일 열기 실패 등)
     *
     * @details
     *   - 이 함수는 "보내기 전에 파일을 패킷 단위로 자르는 기능"에 해당한다.
     *   - 네트워크 전송부에서는 이 벡터를 순회하면서
     *     각 Packet을 인코딩한 뒤 UDP/TCP로 전송하면 된다.
     */
    virtual std::vector<Packet> SplitFile(const std::string& filePath,
                                          std::size_t payloadSize) = 0;

    /**
     * @brief Packet 벡터를 seq 순서대로 정렬하여 하나의 파일로 병합한다.
     *
     * @param outFilePath
     *   병합 결과를 저장할 파일 경로 (예: "output.bin")
     *
     * @param packets
     *   수신된 Packet들을 담은 벡터
     *   - 순서는 섞여 있을 수 있으며, 함수 내부에서 seq 기준으로 정렬한다.
     *
     * @return
     *   - true  : 병합 성공 (파일 생성/쓰기까지 성공)
     *   - false : 실패 (파일 열기 실패, 패킷 데이터 이상 등)
     *
     * @details
     *   - 이 함수는 "수신 측에서 여러 패킷을 다시 원래 파일로 합치는 기능"에 해당한다.
     *   - 네트워크 계층에서 Packet들을 모아 벡터로 넘겨주면,
     *     이 함수가 파일 생성/정렬/쓰기를 대신 처리한다.
     */
    virtual bool MergeFile(const std::string& outFilePath,
                           const std::vector<Packet>& packets) = 0;
};

#endif // IFILE_SPLITTER_AND_MERGER_H
