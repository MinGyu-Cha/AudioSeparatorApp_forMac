#pragma once

#include <QMainWindow>
#include <memory>
#include <QTimer>

class RingBuffer;
class AudioCapture;
class AudioVisualizer;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 코어 파이프라인 포인터 설정 함수
    void setAudioPipeline(std::shared_ptr<RingBuffer> ringBuffer, std::shared_ptr<AudioCapture> capture);

private:
    Ui::MainWindow *ui;
    std::shared_ptr<RingBuffer> m_ringBuffer;
    std::shared_ptr<AudioCapture> m_audioCapture;

    AudioVisualizer* m_visualizer; // 커스텀 UI 위젯
};