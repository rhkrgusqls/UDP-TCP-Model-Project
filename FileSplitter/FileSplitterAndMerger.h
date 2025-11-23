#ifndef FILE_SPLITTER_AND_MERGER_H
#define FILE_SPLITTER_AND_MERGER_H

#include "IFileSplitterAndMerger.h"

/**
 * @brief IFileSplitterAndMerger 인터페이스를 실제로 구현한 클래스
 *
 * - 파일을 payloadSize 크기만큼 잘라 Packet으로 만들고(Split),
 *   다시 Packet들을 이용해 파일로 합치는(Merge) 핵심 로직을 제공한다.
 * - 추가로, 네트워크 전송을 위해 Packet을 문자열로 인코딩/디코딩하는
 *   정적 메서드(EncodePacket, DecodePacket)도 같이 제공한다.
 */
class FileSplitterAndMerger : public IFileSplitterAndMerger {
public:
    FileSplitterAndMerger() = default;
    ~FileSplitterAndMerger() override = default;

    /**
     * @brief 파일을 payloadSize 바이트 단위로 잘라 Packet 벡터를 생성한다.
     *
     * @param filePath    분할할 파일 경로
     * @param payloadSize Packet 하나에 담을 최대 바이트 수
     *
     * @return 분할된 Packet 목록 (실패 시 빈 벡터)
     *
     * @details
     *   - 파일이 10000바이트이고 payloadSize가 1024라면,
     *     대략 10개의 Packet이 만들어진다.
     *   - 각 Packet에는 seq(순번), length(실제 바이트 길이), data(실제내용)가 들어간다.
     */
    std::vector<Packet> SplitFile(const std::string& filePath,
                                  std::size_t payloadSize) override;

    /**
     * @brief Packet 벡터를 seq 순서대로 정렬하여 outFilePath로 병합 저장한다.
     *
     * @param outFilePath 병합된 결과를 저장할 파일 경로
     * @param packets     수신된 Packet 벡터 (seq가 섞여 있어도 됨)
     *
     * @return 병합 성공 시 true, 실패 시 false
     */
    bool MergeFile(const std::string& outFilePath,
                   const std::vector<Packet>& packets) override;

    // ================================================================
    //       네트워크 전송을 위한 Packet <-> 문자열 포맷 변환 유틸
    // ================================================================

    /**
     * @brief Packet 구조체를 "[seq]|[length]{data}" 형식의 문자열로 인코딩한다.
     *
     * @param p  인코딩할 Packet
     * @return   네트워크로 보내기 좋은 문자열 형식
     *
     * 예시)
     *   seq = 0, length = 5, data = "hello" 인 Packet은
     *   "0|5{hello}" 라는 문자열로 변환된다.
     */
    static std::string EncodePacket(const Packet& p);

    /**
     * @brief "[seq]|[length]{data}" 형식의 문자열을 Packet으로 디코딩한다.
     *
     * @param raw  수신한 원본 문자열
     * @param out  디코딩 결과를 저장할 Packet 객체 (출력 매개변수)
     *
     * @return
     *   - true  : 디코딩 성공, out에 올바른 seq/length/data가 채워짐
     *   - false : 형식 오류, 숫자 변환 실패 등
     */
    static bool DecodePacket(const std::string& raw, Packet& out);

    // 패킷 포맷에 사용하는 특수 문자들 (상수)
    // 예: [seq] | [length] { data }
    static constexpr char PACKET_DELIM = '|'; // seq와 length를 구분하는 문자
    static constexpr char PACKET_START = '{'; // data 시작을 알리는 문자
    static constexpr char PACKET_END   = '}'; // data 끝을 알리는 문자
};

#endif // FILE_SPLITTER_AND_MERGER_H
