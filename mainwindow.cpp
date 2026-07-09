#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "RingBuffer.h"
#include "AudioCapture.h"
#include "AudioVisualizer.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    m_visualizer = new AudioVisualizer(this);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_visualizer);
    setCentralWidget(centralWidget);

    resize(800, 400);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setAudioPipeline(std::shared_ptr<RingBuffer> ringBuffer, std::shared_ptr<AudioCapture> capture) {
    m_ringBuffer = ringBuffer;
    m_audioCapture = capture;

    // 오디오 캡처 모듈에게 "이 비주얼라이저 화면에 데이터를 직접 쏴라" 하고 가이드 등록
    if (m_audioCapture) {
        m_audioCapture->setVisualizer(m_visualizer);
    }
}