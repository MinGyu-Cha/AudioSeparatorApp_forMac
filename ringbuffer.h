#pragma once
#include <vector>
#include <atomic>

class RingBuffer {
public:
    // capacity: 버퍼가 담을 수 있는 최대 오디오 샘플 개수
    explicit RingBuffer(size_t capacity);
    ~RingBuffer() = default;

    // Audio Capture 스레드가 호출
    // pcmData: 입력 데이터 포인터, size: 입력할 샘플 개수
    // 실제로 저장된 샘플 개수를 반환
    size_t write(const float* pcmData, size_t size);

    // AI Inference / DSP 스레드가 호출
    // outputBuffer: 데이터를 받아갈 배열 포인터, size: 읽어올 샘플 개수
    // 실제로 읽어온 샘플 개수를 반환
    size_t read(float* outputBuffer, size_t size);

    size_t getAvailableRead() const;
    size_t getAvailableWrite() const;

    // 버퍼 초기화
    void reset();

    size_t peekLatestSamples(float* outputBuffer, size_t size) const;

private:
    std::vector<float> m_buffer;
    size_t m_capacity;

    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;
};
