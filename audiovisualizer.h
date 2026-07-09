// AudioVisualizer.h
#pragma once

#include <QWidget>
#include <vector>
#include <mutex>

class AudioVisualizer : public QWidget {
    Q_OBJECT
public:
    explicit AudioVisualizer(QWidget *parent = nullptr);
    ~AudioVisualizer() = default;

    // 실시간 오디오 콜백 스레드가 이 함수를 통해 안전하게 UI 버퍼로 데이터를 다이렉트 복사합니다.
    void pushAudioData(const float* pcmData, size_t size);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<float> m_displayBuffer; // 화면 렌더링 전용 안전 버퍼
    std::mutex m_mutex;                 // 멀티스레드 데이터 경쟁을 막는 방패
    const size_t m_samplesToDisplay = 512;
};