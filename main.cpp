#include <QCoreApplication>
#include <QDir>
#include <iostream>
#include "AudioDecoder.h"
#include "AudioSeparationEngine.h"

int main(int argc, char *argv[]) {
    // ⚠️ Qt의 경로 인프라 시스템을 가동하기 위해 인스턴스 초기화
    QCoreApplication app(argc, argv);

    std::cout << "=== 파일 기반 AI 음원 분리 파이프라인 테스트 ===" << std::endl;

    QString appDir = QCoreApplication::applicationDirPath();

    // 개발용
    QDir baseDir(appDir);
    baseDir.cdUp();
    baseDir.cdUp();

    QString modelPath = baseDir.filePath("demucs_model.onnx");
    QString testAudioPath = baseDir.filePath("test.mp3");

    //QString modelPath = QDir(baseDir).filePath("demucs_model.onnx");
    std::string modelPathStr = modelPath.toStdString();

    std::cout << "[Main] 도킹할 AI 모델 최종 경로: " << modelPathStr << std::endl;

    //QString testAudioPath = QDir(baseDir).filePath("test.mp3");
    std::vector<float> pcmData = AudioDecoder::decodeToMonoFloatPCM(testAudioPath.toStdString());

    if (pcmData.empty()) {
        std::cerr << "[Main] 오디오 파일(test.mp3) 로드 실패. 실행 파일 옆에 배치해 주세요." << std::endl;
        return -1;
    }

    AudioSeparationEngine engine;
    if (!engine.initEngine(modelPathStr)) {
        return -1;
    }

    engine.processAudioFile(pcmData);

    return 0;
}