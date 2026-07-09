// AudioVisualizer.cpp
#include "AudioVisualizer.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QColor>
#include <algorithm>
#include <cstring>

AudioVisualizer::AudioVisualizer(QWidget *parent)
    : QWidget(parent)
    , m_displayBuffer(512, 0.0f) {
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void AudioVisualizer::pushAudioData(const float* pcmData, size_t size) {
    if (!pcmData || size == 0) return;

    // ⚠️ 스레드 안전성 확보 (Mutex Lock)
    std::lock_guard<std::mutex> lock(m_mutex);

    if (size >= m_samplesToDisplay) {
        // 들어온 데이터가 512개 이상이면 그냥 최신 512개로 싹 교체
        std::memcpy(m_displayBuffer.data(), pcmData + (size - m_samplesToDisplay), m_samplesToDisplay * sizeof(float));
    } else {
        // [핵심] 14개 같이 작은 버퍼가 들어오면 기존 데이터를 앞으로 밀고(Shift) 뒤에 새로 채워 넣음
        std::memmove(m_displayBuffer.data(), m_displayBuffer.data() + size, (m_samplesToDisplay - size) * sizeof(float));
        std::memcpy(m_displayBuffer.data() + (m_samplesToDisplay - size), pcmData, size * sizeof(float));
    }
}

void AudioVisualizer::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 배경 칠하기
    painter.fillRect(rect(), QColor(20, 20, 25));

    int w = width();
    int h = height();
    int midY = h / 2;

    // 중심선 그리기
    painter.setPen(QPen(QColor(60, 60, 70), 1, Qt::DashLine));
    painter.drawLine(0, midY, w, midY);

    // 파형 그리기 (데이터를 읽을 때도 락을 걸어 캡처 스레드가 건드리지 못하게 보호)
    std::vector<float> tempCopy(m_samplesToDisplay, 0.0f);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        tempCopy = m_displayBuffer;
    }

    painter.setPen(QPen(QColor(0, 220, 255), 2));
    double xStep = static_cast<double>(w) / (m_samplesToDisplay - 1);

    for (size_t i = 0; i < m_samplesToDisplay - 1; ++i) {
        int x1 = static_cast<int>(i * xStep);
        int x2 = static_cast<int>((i + 1) * xStep);

        int y1 = midY - static_cast<int>(tempCopy[i] * (midY * 0.8f));
        int y2 = midY - static_cast<int>(tempCopy[i + 1] * (midY * 0.8f));

        y1 = std::max(0, std::min(h, y1));
        y2 = std::max(0, std::min(h, y2));

        painter.drawLine(x1, y1, x2, y2);
    }
}