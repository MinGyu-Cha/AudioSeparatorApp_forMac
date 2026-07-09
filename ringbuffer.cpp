#include "RingBuffer.h"
#include <algorithm>
#include <cstring>

RingBuffer::RingBuffer(size_t capacity)
    : m_capacity(capacity)
    , m_buffer(capacity)
    , m_head(0)
    , m_tail(0) {}

size_t RingBuffer::getAvailableRead() const {
    size_t head = m_head.load(std::memory_order_acquire);
    size_t tail = m_tail.load(std::memory_order_acquire);

    if (head >= tail) {
        return head - tail;
    }
    return m_capacity - tail + head;
}

size_t RingBuffer::getAvailableWrite() const {
    // 읽지 않은 데이터가 버퍼를 덮어쓰지 않도록 공간 계산 (안전장치로 1샘플 여유를 둠)
    return m_capacity - getAvailableRead() - 1;
}

size_t RingBuffer::write(const float* pcmData, size_t size) {
    if (size == 0 || pcmData == nullptr) return 0;

    size_t availableWrite = getAvailableWrite();
    size_t writeSize = std::min(size, availableWrite);

    if (writeSize == 0 || writeSize > m_capacity) return 0;

    size_t head = m_head.load(std::memory_order_relaxed);

    // 남은 공간 계산
    size_t bytesToEnd = m_capacity - head;

    if (writeSize <= bytesToEnd) {
        // 한 번에 복사 가능한 경우
        std::memcpy(&m_buffer[head], pcmData, writeSize * sizeof(float));
        head += writeSize;
    } else {
        // 버퍼 끝을 넘어가서 쪼개어 복사해야 하는 경우
        std::memcpy(&m_buffer[head], pcmData, bytesToEnd * sizeof(float));
        std::memcpy(&m_buffer[0], pcmData + bytesToEnd, (writeSize - bytesToEnd) * sizeof(float));
        head = writeSize - bytesToEnd;
    }

    if (head >= m_capacity) head = 0;

    m_head.store(head, std::memory_order_release);

    return writeSize;
}

size_t RingBuffer::read(float* outputBuffer, size_t size) {
    if (size == 0 || outputBuffer == nullptr) return 0;

    size_t availableRead = getAvailableRead();
    size_t readSize = std::min(size, availableRead);

    if (readSize == 0 || readSize > m_capacity) return 0;

    size_t tail = m_tail.load(std::memory_order_relaxed);
    size_t bytesToEnd = m_capacity - tail;

    if (readSize <= bytesToEnd) {
        std::memcpy(outputBuffer, &m_buffer[tail], readSize * sizeof(float));
        tail += readSize;
    } else {
        std::memcpy(outputBuffer, &m_buffer[tail], bytesToEnd * sizeof(float));
        std::memcpy(outputBuffer + bytesToEnd, &m_buffer[0], (readSize - bytesToEnd) * sizeof(float));
        tail = readSize - bytesToEnd;
    }

    if (tail >= m_capacity) tail = 0;

    m_tail.store(tail, std::memory_order_release);

    return readSize;
}

void RingBuffer::reset() {
    m_head.store(0, std::memory_order_release);
    m_tail.store(0, std::memory_order_release);
}

size_t RingBuffer::peekLatestSamples(float* outputBuffer, size_t size) const {
    if (size == 0 || outputBuffer == nullptr) return 0;

    size_t head = m_head.load(std::memory_order_acquire);

    for (size_t i = 0; i < size; ++i) {
        long long targetIdx = static_cast<long long>(head) - static_cast<long long>(size) + static_cast<long long>(i);
        if (targetIdx < 0) {
            targetIdx += m_capacity;
        }
        outputBuffer[i] = m_buffer[static_cast<size_t>(targetIdx) % m_capacity];
    }
    return size;
}
